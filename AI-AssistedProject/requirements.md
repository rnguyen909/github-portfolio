Functional Requirements:
FR-01: allow users to enter resume data
FR-02: allow users to enter job description data
FR-03: validation for empty text
FR-04: connectivity to Google Gemini API
FR-05: submission to Google Gemini for resume analysis
FR-06: display strengths
FR-07: look for areas of improvement
FR-08: display suggestions for format and writing
FR-09: look for relevant skills and keywords
FR-10: display overall recommendations 
FR-11: display error messages for failed requests and missing keys

Non-Functional Requirements:
NFR-01: be simple and straight forward to use
NFR-02: process requests within a timeframe
NFR-03: API key has to be stored securely 
NFR-04: handle errors and exit gracefully
NFR-05: identifiable variables and consistency in code

Assumptions:
Users have internet for communication with Gemini
Users have text of some sort
Users use suggestions with caution 
Google Gemini API key is available

Project Scope:
Has the options to input text and give feedback for strengths, weaknesses, suggestions, recommendations, and handle errors in a graceful way. 
Since this is a simplistic version of an AI resume analyzer, it doesn't have logins, scoring strengths and weaknesses, and other editing options like the more advanced versions I have seen online.
