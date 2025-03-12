const { log, info, warn, error } = require("console");
const express = require("express");
const app = express();
const moment = require("moment");
const http = require("http");
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server, {
    cors: { 
        origin: "*",
        methods: ["GET", "POST"],
        credentials: true
    }
});

const fs = require("fs");

app.use(express.static("public"));
app.use(express.json());

app.get("/chartData", (req, res) => {
    fs.readFile("data.json", "utf8", (err, jsonString) => {
        if (err || !jsonString.trim()) {
            error("File read failed:", err);
            return res.json({ data: [], msg: "error" });
        }
        try {
            let dataJson = JSON.parse(jsonString);
            res.json({ data: dataJson.data || [], msg: "success" });
        } catch (parseError) {
            error("JSON parse error:", parseError);
            return res.json({ data: [], msg: "error" });
        }
    });
});

// Route lấy dữ liệu từ file data.json
app.get("/data", (req, res) => {
    fs.readFile("data.json", "utf8", (err, jsonString) => {
        if (err || !jsonString.trim()) {
            error("File read failed:", err);
            return res.json({ data: [], msg: "error" });
        }
        try {
            let dataJson = JSON.parse(jsonString);
            res.json({ data: dataJson.data || [], msg: "success" });
        } catch (parseError) {
            error("JSON parse error:", parseError);
            return res.json({ data: [], msg: "error" });
        }
    });
});

io.on("connection", (socket) => {
    log("New connection:", socket.id);
    info(
        "[" + socket.id + "] new connection",
        socket.request.connection.remoteAddress
    );

    socket.on("/esp/envir", (data) => {
        log('From ESP32', data);

        fs.readFile("data.json", "utf8", (err, jsonString) => {
            let dataJson;
            if (err || !jsonString.trim()) {
                error("File read failed or empty, initializing new JSON.");
                dataJson = { data: [] };
            } else {
                try {
                    dataJson = JSON.parse(jsonString);
                } catch (parseError) {
                    error("JSON parse error, resetting file:", parseError);
                    dataJson = { data: [] };
                }
            }

            if (data && typeof data === "object") {
                data.time = moment().format("YYYY-MM-DD HH:mm:ss");
                dataJson.data.push(data);

                fs.writeFile("data.json", JSON.stringify(dataJson, null, 2), (err) => {
                    if (err) {
                        error("Error writing file", err);
                    } else {
                        log("Successfully wrote file");
                    }
                });
            }
        }); 

        io.emit("/web/envir", data);
    });

    socket.on("disconnect", () => {
        error("[" + socket.id + "] disconnect.");
    });

    socket.on("reconnect", () => {
        warn("[" + socket.id + "] reconnect.");
    });
});

server.listen(5000, () => {
    log("server is listening on port 5000");
});
