#!/Users/ephraimsiegfried/.pyenv/versions/3.11.2/bin/python

from wsgiref.handlers import CGIHandler

from flask import Flask, jsonify, request

app = Flask(__name__)


@app.route("/hello", methods=["GET", "POST"])
def hello():
    if request.method == "POST":
        name = request.form["name"]
    else:
        name = request.args.get("name", "Guest")
    return jsonify({"message": f"Hello, {name}!"})


if __name__ == "__main__":
    CGIHandler().run(app)
