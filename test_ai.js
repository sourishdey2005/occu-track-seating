const { GoogleGenerativeAI } = require("@google/generative-ai");

const API_KEY = "AIzaSyCqrC_jq2mSCnyCafg48lE-ybSdrz4LGBM";
const genAI = new GoogleGenerativeAI(API_KEY);

async function listModels() {
  try {
    const fetch = (await import('node-fetch')).default; // If library needs it, though usually included
    // Actually, @google/generative-ai has its own client. 
    // They changed the API recently.
    
    // Attempting a simple request with a known common model name
    const model = genAI.getGenerativeModel({ model: "gemini-1.5-flash-latest" });
    const result = await model.generateContent("test");
    console.log("Success with gemini-1.5-flash-latest!");
  } catch (e) {
    console.error("Failed with gemini-1.5-flash-latest:", e.message);
  }
}

listModels();
