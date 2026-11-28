Problem Statement:
Making a resume is quite difficult because there are so many templates and different ways out there. It can be confusing and even daunting. Applicants have a hard time identifying their weaknessses, skills, and the proper way to phrase sentences in a professional setting. By using this AI Resume Assistant, it will analyze and give improvement suggestions using Google's Gemini

Project Objectives:
Make a resume analysis application. 
Let users be able to enter text within the analysis and description sections. 
Connect Google Gemini's AI model to analyze the text.
Give feedback for strengths, weaknesses, and suggest improvements while identifying relevant keywords.
Use Python and Streamlit in VS Code to develop and test.

Functional Requirements:
1. Must have text area for resume data analysis
2. Must have second section for job description analysis
3. Validation from browser for entered text
4. Send the resume data to Gemini API
5. Display the feedback in sections
6. Handle errors and missing keys 

System Features:
Text Input: users can enter text or copy and paste their resume contents
Job Description Input: users can enter in the job description for analysis
Analysis using AI: Google Gemini analyzes the resume data
Display the Feedback: dispalys feedback for strengths, weaknesses, suggestions, recommendations, and relevant keywords
Handle Errors: display error messages if requests to gemini fail or missing information 

Software Design:
This design of the AI Resume Assistant is simplistic and has only the core features that is needed using Python and Streamlit. 
How it works:
1. Open the application
2. Enter resume data
3. Enter job description if needed
4. Click the analyze button
5. Information is sent to Google's Gemini API
6. Gemini analyzes the data
7. Streamlit displays results 

Technology Stack:
Python - programming language used in vs code
Streamlit - user interface
Google Gemini API - analysis using gemini
Google Gen AI SDK - communication with gemini
Python-dotenv - API key 
Git - versioning
GitHUb - where the source code is

Development Timeline:
1. Pick a topic and look at what is needed to be done
2. Configure the environment for the code
3. Work on Streamlit application interface 
4. Connect Gemini API 
5. Test for any errors
6. Finish all documents 
7. Take screenshots
8. Submit the project with all requirements
