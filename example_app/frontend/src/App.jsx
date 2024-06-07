import { useState, useEffect } from "react";
import "./App.css";
import kitty1 from "./assets/kitty1.jpg";
import rat from "./assets/rat.jpg";

function App() {
    const [name, setName] = useState("");
    const [message, setMessage] = useState("");
    const [submitting, setSubmitting] = useState(false);
    const [formError, setFormError] = useState("");
    const [data, setData] = useState([]);
    const POLLING_INTERVAL = 2000;

    useEffect(() => {
        // Fetch messages when the component mounts and on every interval
        const fetchMessages = async () => {
            try {
                const startIndex = data.length + 1;
                const response = await fetch(
                    `/api/messages?startIndex=${startIndex}`,
                );
                if (!response.ok) throw new Error("Failed to fetch");
                const newMessages = await response.json();
                setData((prevData) => [...prevData, ...newMessages]);
            } catch (error) {
                setFormError(error.toString());
            }
        };

        fetchMessages(); // Initial fetch

        const intervalId = setInterval(() => {
            fetchMessages();
        }, POLLING_INTERVAL);

        return () => clearInterval(intervalId);
    }, [data.length]);

    const onFormSubmit = async (e) => {
        e.preventDefault();
        setFormError("");
        setSubmitting(true);

        try {
            const newData = { name, message };
            const response = await fetch("/api/messages", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(newData),
            });
            if (!response.ok) throw new Error("Failed to post message");
            const postedMessage = await response.json();
            setData([...data, postedMessage]); // Update local state with the new message
            setMessage("");
        } catch (err) {
            console.error(err);
            setFormError(err.toString());
        } finally {
            setSubmitting(false);
        }
    };

    return (
        <>
            <h1>THE BEST WEBSITE EVER</h1>
            <h4>Lets talk about cats. What do you like about cats?</h4>
            {data.map((submission, index) => {
                return (
                    <div key={index}>
                        <p>
                            <b>{submission.name} says:</b> {submission.message}
                        </p>
                    </div>
                );
            })}
            <form onSubmit={onFormSubmit}>
                <input
                    type="text"
                    placeholder="Your name"
                    value={name}
                    onChange={(e) => setName(e.currentTarget.value)}
                    required
                />
                <input
                    type="text"
                    placeholder="Say something nice about kitty cat"
                    value={message}
                    onChange={(e) => setMessage(e.currentTarget.value)}
                    required
                />
                {Boolean(formError) && (
                    <div className="form-error">{formError}</div>
                )}
                <button type="submit" disabled={submitting}>
                    Submit
                </button>
            </form>
            <img src={kitty1} height={500} />
            <img src={rat} height={300} className="rotate" />
        </>
    );
}

export default App;
