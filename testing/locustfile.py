# import time
from locust import HttpUser, task, between
from lorem_text import lorem
import random

class QuickstartUser(HttpUser):
    wait_time = between(0.1, 0.5)

    @task
    def get_messages(self):
        # https://kitty.qew.ch/api/messages?asd=1
        self.client.get("/api/messages?asd=1")

    @task
    def send_message(self):
        words = random.randint(1, 10)
        payload = {
            "name": "rahel", 
            "message": lorem.words(words)
        }
        self.client.post(f"/messages", json=payload, )
        # time.sleep(1)

