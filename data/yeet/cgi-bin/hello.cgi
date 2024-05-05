#!/Users/ephraimsiegfried/.pyenv/versions/3.11.2/bin/python

import json
import os
from wsgiref.handlers import CGIHandler

from flask import Flask, jsonify, request

app = Flask(__name__)
DATA_FILE = "data.json"


@app.route("/api/messages", methods=["GET", "POST"])
def message_handler():
    if request.method == "POST":
        # Handle POST request
        data = request.json
        if not data or "name" not in data or "message" not in data:
            return (
                jsonify(
                    {
                        "error": "Bad Request",
                        "message": "Please provide both name and message",
                    }
                ),
                400,
            )

        # Save the received data to the JSON file
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, "r+") as file:
                file_data = json.load(file)
                file_data.append(data)
                file.seek(0)
                json.dump(file_data, file)
        else:
            with open(DATA_FILE, "w") as file:
                json.dump([data], file)

        return data, 201

    elif request.method == "GET":
        # Handle GET request
        if not os.path.exists(DATA_FILE):
            return jsonify([])  # Return empty list if file does not exist

        with open(DATA_FILE, "r") as file:
            file_data = json.load(file)
            return jsonify(file_data)


if __name__ == "__main__":
    CGIHandler().run(app)
