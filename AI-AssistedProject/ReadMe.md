Project Overview:
In this project, the application uses a simple interface with barebones basic functions. By connecting to Google Gemini's AI, we are able to analyze resume data to give feedback on various sections such as strengths, weaknesses, and relevant keywords for the user's input data. 

Features:
Text input
Job description input
Resume analysis
Look for strengths and weaknesses
Provide suggestions
Recommendation for relevant skills and keywords
Overall recommendations 
Error and validation handling


Installation:
1. Clone repository from github
git clone https://github.com/rnguyen909/github-portfolio.git

2. Go to project folder
cd github-portfolio/AI-AssistedProject

3. Configure/Create environment  
python -m venv .venv

4. Activate environment through powershell
.\.venv\Scripts\Activate.ps1

5. Install dependencies
pip install -r requirements.txt

6. Create .env file to store api key
GEMINI_API_KEY=your_api_key_here

Usage Guide:
To start the application, go to microsoft visual studio code and enter this into the terminal:
python -m streamlit run app.py

From there it should pop up the application in your local browser.

Screenshots:

![alt text](<Screenshot 2026-11-27 182516.png>)
![alt text](<Screenshot 2026-11-27 183001.png>)
![alt text](<Screenshot 2026-11-27 183027.png>)
![alt text](<Screenshot 2026-11-27 183054.png>)
![alt text](<Screenshot 2026-11-27 183122.png>)
![alt text](<Screenshot 2026-11-27 183143.png>)

Technologies Used:
Python
Streamlit
Google Gemini API
Google Gen AI SDK
Python-dotenv
Github and Git

Future Enhancements:
After getting suggestions and recommendations, the application should automatically rewrite the data. 
Be able to download a version of the resume that is edited from the suggestions and recommendations. 
Expand on the feedback to be more detailed. 
Provide a cleaner interface that is more pleasing to the eye with nice colors and graphics. 
Have a login option to be able to store resumes. 

