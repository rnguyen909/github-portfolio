#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <sqlite3.h>


using namespace std;


struct Participant {
    int id{};
    string name;
    int losses = 0;
    bool eliminated = false;
};


struct MatchOutcome {
    Participant* winner = nullptr;
    Participant* loser = nullptr;
    int score1 = 0;
    int score2 = 0;
    string note;
    bool playedByCoins = false;
};


class Tournament {
private:
    sqlite3* db = nullptr;
    string tournamentName;
    int participantLimit = 0;
    vector<Participant> participants; // Reserve before adding so pointers remain stable.
    vector<Participant*> mainBracket;
    vector<Participant*> losersBracket;
    mt19937 rng{static_cast<unsigned int>(chrono::steady_clock::now().time_since_epoch().count())};
    int nextParticipantId = 1;
    bool tournamentStarted = false;


    static string trim(const string& value) {
        const size_t first = value.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        const size_t last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }


    string readLine(const string& prompt) {
        cout << prompt << flush;
        string line;
        if (!getline(cin, line)) {
            throw runtime_error("Input ended before the tournament was complete.");
        }
        return trim(line);
    }


    int readInt(const string& prompt, int minValue, int maxValue) {
        while (true) {
            string line = readLine(prompt);
            stringstream ss(line);
            int value;
            char extra;
            if ((ss >> value) && !(ss >> extra) && value >= minValue && value <= maxValue)
                return value;
            cout << "Invalid input. Enter a whole number from " << minValue
                 << " to " << maxValue << ". Please try again.\n";
        }
    }


    bool openDatabase() {
        if (sqlite3_open("tournament.db", &db) != SQLITE_OK) {
            cerr << "Database error: " << (db ? sqlite3_errmsg(db) : "could not open database") << '\n';
            return false;
        }
        const char* schema =
            "PRAGMA foreign_keys = ON;"
            "CREATE TABLE IF NOT EXISTS tournaments ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, participant_limit INTEGER NOT NULL, created_at TEXT DEFAULT CURRENT_TIMESTAMP);"
            "CREATE TABLE IF NOT EXISTS participants ("
            " id INTEGER NOT NULL, tournament_id INTEGER NOT NULL, name TEXT NOT NULL, PRIMARY KEY(tournament_id, id),"
            " FOREIGN KEY(tournament_id) REFERENCES tournaments(id));"
            "CREATE TABLE IF NOT EXISTS matches ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT, tournament_id INTEGER NOT NULL, round_name TEXT NOT NULL,"
            " player1 TEXT NOT NULL, player2 TEXT NOT NULL, score1 INTEGER NOT NULL, score2 INTEGER NOT NULL,"
            " winner TEXT NOT NULL, result_note TEXT NOT NULL, recorded_at TEXT DEFAULT CURRENT_TIMESTAMP,"
            " FOREIGN KEY(tournament_id) REFERENCES tournaments(id));"
            "CREATE TRIGGER IF NOT EXISTS matches_no_update BEFORE UPDATE ON matches BEGIN"
            " SELECT RAISE(ABORT, 'Recorded match results cannot be edited'); END;"
            "CREATE TRIGGER IF NOT EXISTS matches_no_delete BEFORE DELETE ON matches BEGIN"
            " SELECT RAISE(ABORT, 'Recorded match results cannot be deleted'); END;";
        char* error = nullptr;
        if (sqlite3_exec(db, schema, nullptr, nullptr, &error) != SQLITE_OK) {
            cerr << "Database setup error: " << (error ? error : "unknown error") << '\n';
            sqlite3_free(error);
            return false;
        }
        return true;
    }


    bool executeSql(const string& sql) {
        char* error = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
            cerr << "Database error: " << (error ? error : "unknown error") << '\n';
            sqlite3_free(error);
            return false;
        }
        return true;
    }


    int saveTournamentRecord() {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO tournaments(name, participant_limit) VALUES(?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Could not prepare tournament record.\n";
            return -1;
        }
        sqlite3_bind_text(stmt, 1, tournamentName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, participantLimit);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Could not save tournament: " << sqlite3_errmsg(db) << '\n';
            sqlite3_finalize(stmt);
            return -1;
        }
        sqlite3_finalize(stmt);
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }


    bool saveParticipant(const Participant& p, int tournamentId) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO participants(id, tournament_id, name) VALUES(?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int(stmt, 1, p.id);
        sqlite3_bind_int(stmt, 2, tournamentId);
        sqlite3_bind_text(stmt, 3, p.name.c_str(), -1, SQLITE_TRANSIENT);
        const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        if (!ok) cerr << "Could not save participant: " << sqlite3_errmsg(db) << '\n';
        sqlite3_finalize(stmt);
        return ok;
    }


    bool saveMatch(const string& roundName, Participant* a, Participant* b,
                   const MatchOutcome& outcome, int tournamentId) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO matches(tournament_id, round_name, player1, player2, score1, score2, winner, result_note) VALUES(?, ?, ?, ?, ?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Could not prepare match record.\n";
            return false;
        }
        sqlite3_bind_int(stmt, 1, tournamentId);
        sqlite3_bind_text(stmt, 2, roundName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, a->name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, b->name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, outcome.score1);
        sqlite3_bind_int(stmt, 6, outcome.score2);
        sqlite3_bind_text(stmt, 7, outcome.winner->name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, outcome.note.c_str(), -1, SQLITE_TRANSIENT);
        const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        if (!ok) cerr << "Could not save match: " << sqlite3_errmsg(db) << '\n';
        sqlite3_finalize(stmt);
        return ok;
    }


    void registerParticipants(int tournamentId) {
        int count = readInt("How many participants? (2-64): ", 2, 64);
        participantLimit = count;
        participants.reserve(static_cast<size_t>(count));
        cout << "\nEnter a unique name for each participant.\n";
        while (static_cast<int>(participants.size()) < count) {
            string name = readLine("Participant " + to_string(participants.size() + 1) + " name: ");
            if (name.empty()) {
                cout << "Name cannot be blank. Please try again.\n";
                continue;
            }
            bool duplicate = false;
            for (const auto& p : participants) {
                if (p.name == name) { duplicate = true; break; }
            }
            if (duplicate) {
                cout << "That name is already registered. Enter a different name.\n";
                continue;
            }
            Participant p;
            p.id = nextParticipantId++;
            p.name = name;
            participants.push_back(p);
            if (!saveParticipant(participants.back(), tournamentId)) {
                cout << "Warning: participant could not be saved to the database.\n";
            }
        }
        for (auto& p : participants) mainBracket.push_back(&p);
    }


    void showBracket(const vector<Participant*>& bracket, const string& title) {
        cout << "\n--- " << title << " ---\n";
        if (bracket.empty()) {
            cout << "(No participants)\n";
            return;
        }
        for (size_t i = 0; i < bracket.size(); ++i) {
            cout << (i + 1) << ". " << bracket[i]->name
                 << " [losses: " << bracket[i]->losses << "]\n";
        }
    }


    void showAllBrackets() {
        showBracket(mainBracket, "MAIN / WINNERS BRACKET");
        showBracket(losersBracket, "LOSERS BRACKET");
    }


    MatchOutcome playMatch(Participant* a, Participant* b, const string& roundName,
                           int tournamentId, bool eliminateMatchLoser = false) {
        MatchOutcome out;
        cout << "\n=== " << roundName << " ===\n";
        cout << a->name << " vs " << b->name << '\n';
        cout << "Organizer: choose match status: 0 = normal match, 1 = " << a->name
             << " forfeits, 2 = " << b->name << " forfeits, 3 = disqualify " << a->name
             << ", 4 = disqualify " << b->name << ".\n";
        int status = readInt("Selection: ", 0, 4);
        if (status != 0) {
            Participant* removed = (status == 1 || status == 3) ? a : b;
            Participant* winner = (removed == a) ? b : a;
            removed->eliminated = true;
            out.winner = winner;
            out.loser = removed;
            out.note = (status <= 2 ? "forfeit: " : "disqualification: ") + removed->name;
            cout << removed->name << " is removed; " << winner->name << " advances automatically.\n";
        } else {
            out.playedByCoins = true;
            while (out.score1 < 2 && out.score2 < 2) {
                uniform_int_distribution<int> coin(0, 1);
                const int flip = coin(rng);
                if (flip == 0) ++out.score1;
                else ++out.score2;
                cout << "Coin flip: " << (flip == 0 ? a->name : b->name)
                     << " wins. Score " << a->name << " " << out.score1
                     << " - " << out.score2 << " " << b->name << '\n';
            }
            out.winner = (out.score1 == 2) ? a : b;
            out.loser = (out.winner == a) ? b : a;
            out.note = "best-of-three coin flip";
            cout << "Match winner: " << out.winner->name << '\n';
        }


        if (!out.loser->eliminated) {
            if (eliminateMatchLoser) {
                out.loser->eliminated = true;
            } else {
                ++out.loser->losses;
                if (out.loser->losses >= 2) out.loser->eliminated = true;
            }
        }
        saveMatch(roundName, a, b, out, tournamentId);
        return out;
    }


    void placeMainLoser(MatchOutcome& outcome) {
        Participant* loser = outcome.loser;
        if (!loser->eliminated && loser->losses == 1) losersBracket.push_back(loser);
        else if (loser->eliminated) cout << loser->name << " is eliminated from the tournament.\n";
    }


    void runLosersRound(int tournamentId, const string& label) {
        if (losersBracket.size() < 2) return;
        shuffle(losersBracket.begin(), losersBracket.end(), rng);
        vector<Participant*> survivors;
        for (size_t i = 0; i + 1 < losersBracket.size(); i += 2) {
            Participant* a = losersBracket[i];
            Participant* b = losersBracket[i + 1];
            MatchOutcome result = playMatch(a, b, label, tournamentId);
            if (!result.winner->eliminated) survivors.push_back(result.winner);
            if (result.loser->eliminated) cout << result.loser->name << " is eliminated.\n";
        }
        if (losersBracket.size() % 2 != 0) {
            Participant* bye = losersBracket.back();
            if (!bye->eliminated) {
                survivors.push_back(bye);
                cout << bye->name << " receives a loser's-bracket bye.\n";
            }
        }
        losersBracket = survivors;
    }


    void handleOddParticipant(Participant* unmatched, int tournamentId) {
        if (losersBracket.empty()) {
            cout << unmatched->name << " advances automatically; no eligible loser-bracket opponent is available.\n";
            mainBracket.push_back(unmatched);
            return;
        }
        uniform_int_distribution<size_t> pick(0, losersBracket.size() - 1);
        size_t index = pick(rng);
        Participant* opponent = losersBracket[index];
        losersBracket.erase(losersBracket.begin() + static_cast<ptrdiff_t>(index));
        cout << "\nFR-09 odd-participant match: " << unmatched->name << " vs " << opponent->name << '\n';
        MatchOutcome result = playMatch(unmatched, opponent, "Odd-participant match", tournamentId, true);
        if (!result.winner->eliminated) mainBracket.push_back(result.winner);
        cout << "The loser of the odd-participant match is eliminated as required by FR-09.\n";
    }


public:
    ~Tournament() { if (db) sqlite3_close(db); }


    int run() {
        if (!openDatabase()) return 1;
        cout << "====================================\n"
             << " C++ COIN-FLIP TOURNAMENT SYSTEM\n"
             << "====================================\n";
        tournamentName = readLine("Enter tournament name: ");
        while (tournamentName.empty()) {
            cout << "Tournament name cannot be blank.\n";
            tournamentName = readLine("Enter tournament name: ");
        }
        // Limit is asked separately because it is a tournament setting; the count must not exceed it.
        participantLimit = readInt("Set participant limit (2-64): ", 2, 64);
        int tournamentId = saveTournamentRecord();
        if (tournamentId < 0) return 1;
        int count = readInt("How many participants are registering? (2-" + to_string(participantLimit) + "): ", 2, participantLimit);
        participants.reserve(static_cast<size_t>(participantLimit));
        cout << "Enter unique participant names.\n";
        while (static_cast<int>(participants.size()) < count) {
            string name = readLine("Participant " + to_string(participants.size() + 1) + ": ");
            if (name.empty()) { cout << "Name cannot be blank; try again.\n"; continue; }
            bool duplicate = any_of(participants.begin(), participants.end(), [&](const Participant& p) { return p.name == name; });
            if (duplicate) { cout << "Name already exists; use a unique name.\n"; continue; }
            Participant p; p.id = nextParticipantId++; p.name = name;
            participants.push_back(p);
            if (!saveParticipant(participants.back(), tournamentId)) cout << "Warning: database save failed for this participant.\n";
        }


        // FR-08: shuffle once to generate randomized first-round matchups.
        const auto startTime = chrono::steady_clock::now();
        for (auto& p : participants) mainBracket.push_back(&p);
        shuffle(mainBracket.begin(), mainBracket.end(), rng);
        tournamentStarted = true;
        cout << "\nRandomized bracket generated.\n";
        showAllBrackets();
        const auto generatedMs = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count();
        cout << "Bracket generation time: " << generatedMs << " ms\n";


        int mainRound = 1;
        Participant* mainChampion = nullptr;
        bool oddRuleApplied = false;
        while (mainBracket.size() > 1) {
            vector<Participant*> current = mainBracket;
            vector<Participant*> nextRound;
            Participant* unmatched = nullptr;
            if (current.size() % 2 != 0) {
                unmatched = current.back();
                current.pop_back();
            }
            cout << "\n######## MAIN BRACKET ROUND " << mainRound << " ########\n";
            for (size_t i = 0; i + 1 < current.size(); i += 2) {
                MatchOutcome result = playMatch(current[i], current[i + 1], "Main round " + to_string(mainRound), tournamentId);
                if (!result.winner->eliminated) nextRound.push_back(result.winner);
                placeMainLoser(result);
            }
            if (unmatched) {
                if (mainRound == 1 && !oddRuleApplied) {
                    handleOddParticipant(unmatched, tournamentId);
                    oddRuleApplied = true;
                    // handleOddParticipant adds its winner (or the unmatched participant) to mainBracket.
                    for (Participant* p : mainBracket) {
                        if (find(current.begin(), current.end(), p) == current.end() &&
                            find(nextRound.begin(), nextRound.end(), p) == nextRound.end() && !p->eliminated) {
                            nextRound.push_back(p);
                        }
                    }
                } else {
                    cout << unmatched->name << " receives a main-bracket bye.\n";
                    nextRound.push_back(unmatched);
                }
            }
            mainBracket = nextRound;
            runLosersRound(tournamentId, "Losers round after main round " + to_string(mainRound));
            showAllBrackets();
            ++mainRound;
            if (mainBracket.empty()) break; // All main-bracket participants were forfeited/disqualified.
        }


        if (mainBracket.size() == 1) mainChampion = mainBracket.front();
        if (!mainChampion) {
            cout << "No main-bracket champion could be determined. Check forfeits/disqualifications.\n";
            if (db) sqlite3_close(db);
            db = nullptr;
            return 0;
        }


        // Finish remaining losers-bracket rounds until there is one survivor.
        int extraLosersRound = 1;
        while (losersBracket.size() > 1) {
            runLosersRound(tournamentId, "Final losers-bracket round " + to_string(extraLosersRound++));
        }
        Participant* losersChampion = losersBracket.empty() ? nullptr : losersBracket.front();
        cout << "\nMain-bracket winner: " << mainChampion->name << '\n';
        if (!losersChampion || losersChampion == mainChampion || losersChampion->eliminated) {
            cout << "No eligible loser's-bracket finalist remains.\n";
            cout << "TOURNAMENT CHAMPION: " << mainChampion->name << '\n';
        } else {
            cout << "Loser's-bracket winner: " << losersChampion->name << '\n';
            cout << "\n######## GRAND FINAL ########\n";
            MatchOutcome firstFinal = playMatch(mainChampion, losersChampion, "Grand final", tournamentId, false);
            Participant* champion = firstFinal.winner;
            // Only a coin-played win by the loser's-bracket finalist triggers a reset.
            if (firstFinal.playedByCoins && champion == losersChampion) {
                cout << "Loser's-bracket winner won the first grand final. Reset match required.\n";
                // In the reset match, the participant losing their second match is eliminated.
                MatchOutcome reset = playMatch(mainChampion, losersChampion, "Grand final reset", tournamentId, false);
                champion = reset.winner;
            }
            cout << "\n====================================\n"
                 << " TOURNAMENT CHAMPION: " << champion->name << "\n"
                 << "====================================\n";
        }
        cout << "Tournament results saved to tournament.db\n";
        cout << "Note: match rows are protected from UPDATE and DELETE by SQLite triggers.\n";
        return 0;
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    try {
        Tournament tournament;
        return tournament.run();
    } catch (const exception& ex) {
        cerr << "Unexpected error: " << ex.what() << '\n';
        return 1;
    }
}
