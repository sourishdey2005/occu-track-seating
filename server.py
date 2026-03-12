
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import random

app = FastAPI()

# 15 seats simulation
seats = {i: "FREE" for i in range(1, 16)}

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/update")
async def update_seat(data: dict):
    seat_id = data["seat_id"]
    status = data["status"]
    seats[seat_id] = status

    # Simulate other seats randomly
    for i in range(2, 16):
        seats[i] = random.choice(["FREE", "PARTIAL", "OCCUPIED"])

    return {"message": "updated"}

@app.get("/seats")
def get_seats():
    return seats
