
import os
import streamlit as st
from dotenv import load_dotenv
from google import genai

load_dotenv()

st.set_page_config(
    page_title="AI Resume Assistant",
    page_icon="📄",
    layout="centered"
)

st.title("📄 AI Resume Assistant")

st.write(
    "Get AI-powered feedback on your resume "
    "and discover ways to improve it."
)

resume = st.text_area(
    "Paste your resume here:",
    height=250,
    placeholder="Paste your resume text..."
)

job_description = st.text_area(
    "Paste a job description (optional):",
    height=150,
    placeholder="Paste a job description here..."
)

if st.button("Analyze Resume"):
    if not resume.strip():
        st.warning("Please paste your resume first.")

    else:
        api_key = os.getenv("GEMINI_API_KEY")

        if not api_key:
            st.error("Gemini API key not found. Check your .env file.")

        else:
            prompt = f"""
            Act as a professional resume reviewer.

            Analyze the following resume.

            Resume:
            {resume}

            Job Description:
            {job_description if job_description.strip() else "Not provided."}

            Provide your feedback using these sections:

            1. Resume Strengths
            2. Areas for Improvement
            3. Writing and Formatting Suggestions
            4. Relevant Skills and Keywords
            5. Overall Recommendations

            If no job description is provided, focus on general
            resume improvements.

            Do not invent qualifications or experience.
            """

            try:
                with st.spinner("Gemini is analyzing your resume..."):
                    with genai.Client(api_key=api_key) as client:
                        response = client.models.generate_content(
                            model="gemini-3.8-flash",
                            contents=prompt
                        )

                st.subheader("Resume Analysis")
                st.markdown(response.text)

            except Exception as error:
                st.error(f"An error occurred: {error}")
