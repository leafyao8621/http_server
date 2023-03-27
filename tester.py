import requests
import time

if __name__ == "__main__":
    for i in range(10):
        for j in range(10):
            payload = {"a": i, "b": j}
            res = requests.post("http://localhost:8000/add", json=payload)
            print(res.json())
            res.close()
            time.sleep(0.4)
