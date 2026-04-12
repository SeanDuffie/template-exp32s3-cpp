<script lang="ts">
    import { onMount, tick } from 'svelte';

    // -- App State --
    let currentPage = $state('home'); // 'home', 'console', 'files'
    let isDarkMode = $state(true);
    let terminalContainer: HTMLElement | undefined = $state<HTMLElement>();

    // -- Session Data --
    let sessionData = $state({
        systemTime: "...",  // What is the current time?
        startDate: "2026-03-01",  // When was the current collection started?
        estimatedEndDate: "2026-05-01",  // When will the collection be finished?
        daysElapsed: 25,  // How long has the collection been running?
        upsec: 0,
        uptime: "0s",  // How long since the last powercycle?
        terminalLogs: "...\n",  // Console logs
    });

    // -- Hardware Configuration Flags --
    // Toggle these to true/false based on what is physically connected
    let activeSensors = $state({
        ds18b20: false,
        soil: false,
        bme280: false,
        bh1750: false, // Set to true to see the card
        gps: true,
        imu: false     // Set to true to see the card
    });

    // -- Sensor Data --
    let liveData = $state({
        // probeTempF: "", // °F
        // probeTempC: "", // °C
        // soilMoisture: "",  // %
        // soilMoistureRaw: "", // ADC
        // airTempF: "", // °F
        // airTempC: "", // °C
        // humidity: "",  // %
        // pressure: "",  // hPa
        // lightLux: "",  // Lumen
        gpsLat: "",
        gpsLon: "",
        gpsAlt: "",
        gpsSpeed: "",
        gpsSats: "",
        // imuPitch: "",
        // imuRoll: "",
    });

    // -- Filesystem Data --
    // Define the shape of the data coming from the ESP32
    interface FileData {
        name: string;
        size: number;
    }
    let fileList = $state<FileData[]>([]);
    
    // // System States
    // let isUpdating = $state(false);
    // let wifiSSID = $state("");
    // let wifiPass = $state("");
    // let wifiStatusMessage = $state("");
    
    // async function handleUpdate() {
    //     isUpdating = true;
    //     try {
    //         // Replace with actual ESP32 API endpoint
    //         const res = await fetch('/api/update', { method: 'POST' });
    //         if (res.ok) alert("Update triggered. Rebooting.");
    //         else alert("Update failed.");
    //     } catch (e) {
    //         alert("Network error.");
    //     }
    //     isUpdating = false;
    // }
    
    // async function handleWifiUpdate() {
    //     if (!wifiSSID) {
    //         wifiStatusMessage = "SSID cannot be blank.";
    //         return;
    //     }
    //     wifiStatusMessage = "Sending credentials...";
    //     try {
    //         // Replace with actual ESP32 API endpoint
    //         const res = await fetch('/api/wifi', {
    //             method: 'POST',
    //             headers: { 'Content-Type': 'application/json' },
    //             body: JSON.stringify({ ssid: wifiSSID, password: wifiPass })
    //         });
    //         if (res.ok) wifiStatusMessage = "Credentials saved. Rebooting.";
    //         else wifiStatusMessage = "Failed to update credentials.";
    //     } catch (e) {
    //         wifiStatusMessage = "Network error.";
    //     }
    // }

    async function loadFiles() {
        try {
            const res = await fetch('/api/files');
            if (res.ok) {
                const data = await res.json();
                fileList = data.files;
            } else {
                console.error("Failed to fetch files");
            }
        } catch (e) {
            console.error("Error fetching files");
        }
    }

    // Handle theme swapping logic
    function toggleTheme() {
        isDarkMode = !isDarkMode;
        if (isDarkMode) {
            document.body.classList.add('dark');
        } else {
            document.body.classList.remove('dark');
        }
    }

    // Replace DOM contents with new page
    function navigate(page: string) {
        currentPage = page;
        if (page === 'files') {
            loadFiles();
        }
    }

    // Auto-scroll terminal when new logs arrive
    $effect(() => {
        // This runs automatically whenever currentPage or terminalContainer changes
        if (currentPage === 'console' && terminalContainer) {
            terminalContainer.scrollTop = terminalContainer.scrollHeight;
        }
    });

    onMount(() => {
        // Establish connection to ESP32
        const socket = new WebSocket(`ws://${window.location.hostname}/ws`);

        socket.onmessage = async (event) => {
            try {
                const data = JSON.parse(event.data);

                // Map JSON keys to your UI variables
                sessionData.systemTime = data.timestamp;
                sessionData.upsec = Number(data.uptime);
                sessionData.uptime = new Date(sessionData.upsec * 1000).toISOString().slice(11, 19);
                
                // Update active sensor cards dynamically
                if (data.active_sensors) {
                    // activeSensors.ds18b20 = data.active.ds18b20;
                    // activeSensors.soil = data.active.soil;
                    // activeSensors.bme280 = data.active.bme280;
                    // activeSensors.bh1750 = data.active.bh1750;
                    // activeSensors.gps = data.active.gps;
                    // activeSensors.imu = data.active.imu;
                    activeSensors = data.active_sensors;
                }

                // if (activeSensors.ds18b20) {
                //     liveData.probeTempF = data.bin.probeTempF.toFixed(2);
                //     liveData.probeTempC = data.bin.probeTempC.toFixed(2);
                // }

                // if (activeSensors.soil) {
                //     liveData.soilMoisture = data.bin.soilMoisture.toFixed(2);
                //     liveData.soilMoistureRaw = data.bin.binMoistRaw;
                // }

                // if (activeSensors.bme280) {
                //     liveData.airTempF = data.env.airTempF.toFixed(2);
                //     liveData.airTempC = data.env.airTempC.toFixed(2);
                //     liveData.humidity = data.env.airHumidity.toFixed(2);
                //     liveData.pressure = data.env.airPress.toFixed(2);
                // }

                // if (activeSensors.bh1750) {
                //     liveData.lightLux = data.env.lightLux;
                // }

                if (activeSensors.gps) {
                    liveData.gpsLat = data.gps.latitude;
                    liveData.gpsLon = data.gps.longitude;
                    liveData.gpsAlt = data.gps.altitude;
                    liveData.gpsSpeed = data.gps.speed;
                    liveData.gpsSats = data.gps.satellites;
                }

                // if (activeSensors.imu) {
                //     liveData.imuPitch = data.imu.pitch;
                //     liveData.imuRoll = data.imu.roll;
                // }
                
                // Append logs to terminal
                if (data.logs) {
                    sessionData.terminalLogs += data.logs;

                    // Only attempt to scroll if the console page is currently visible
                    if (currentPage === 'console' && terminalContainer) {
                        await tick(); // Wait for the DOM to update with the new text
                        terminalContainer.scrollTop = terminalContainer.scrollHeight;
                    }
                }

            } catch (err) {
                console.error("Failed to parse sensor JSON", err);
            }
        }
        return () => socket.close();
    });

</script>

<!-- <body class="dark"> -->
    <main class="app-container">
        <header>
            <h1>Sensor Node Dashboard</h1>
            <span class="status-indicator">●</span>
        </header>

        <nav class="navbar">
            <div class="nav-links">
                <button class:active={currentPage === 'home'} onclick={() => navigate('home')}>Dashboard</button>
                <button class:active={currentPage === 'files'} onclick={() => navigate('files')}>SD Card</button>
                <button class:active={currentPage === 'console'} onclick={() => navigate('console')}>Console</button>
                <button onclick={toggleTheme}>{isDarkMode ? '☀️ Light' : '🌙 Dark'}</button>
            </div>
        </nav>

        <div class="card system-stats">
            <div class="stat"><span class="label">System Time:</span> {sessionData.systemTime}</div>
            <div class="stat"><span class="label">Uptime:</span> {sessionData.uptime}</div>
            <!-- <div class="stat"><span class="label">Status:</span> <span style="color: #16a34a;">●</span></div> -->
        </div>

        {#if currentPage === 'home'}
            <div class="dashboard grid-container">
                <!-- DS18B20 -->
                <!-- {#if activeSensors.ds18b20}
                <section class="card">
                    <h2>Temperature Probe (DS18B20)</h2>
                    <div class="reading"><span class="label">Temperature</span><span class="value">{liveData.probeTempF}°F</span></div>
                </section>
                {/if} -->

                <!-- Soil Moisture -->
                <!-- {#if activeSensors.soil}
                <section class="card">
                    <h2>Capacitive Soil Moisture Sensor (Analog)</h2>
                    <div class="reading"><span class="label">Moisture Level</span><span class="value">{liveData.soilMoisture}%</span></div>
                    <div class="reading"><span class="label">Raw ADC</span><span class="value">{liveData.soilMoistureRaw}</span></div>
                </section>
                {/if} -->

                <!-- BME280 -->
                <!-- {#if activeSensors.bme280}
                <section class="card">
                    <h2>Air Quality Sensor (BME280)</h2>
                    <div class="reading"><span class="label">Air Temp</span><span class="value">{liveData.airTempF}°F</span></div>
                    <div class="reading"><span class="label">Humidity</span><span class="value">{liveData.humidity}%</span></div>
                    <div class="reading"><span class="label">Pressure</span><span class="value">{liveData.pressure} hPa</span></div>
                </section>
                {/if} -->

                <!-- BH1750 -->
                <!-- {#if activeSensors.bh1750}
                <section class="card">
                    <h2>Lux Sensor (BH1750)</h2>
                    <div class="reading"><span class="label">Intensity</span><span class="value">{liveData.lightLux} Lux</span></div>
                </section>
                {/if} -->

                <!-- NEO-6M GPS -->
                {#if activeSensors.gps}
                <section class="card">
                    <h2>GPS Location Module (NEO-6M)</h2>
                    <div class="reading"><span class="label">Latitude</span><span class="value">{liveData.gpsLat}</span></div>
                    <div class="reading"><span class="label">Longitude</span><span class="value">{liveData.gpsLon}</span></div>
                    <div class="reading"><span class="label">Altitude</span><span class="value">{liveData.gpsAlt}</span></div>
                    <div class="reading"><span class="label">Speed</span><span class="value">{liveData.gpsSpeed}</span></div>
                    <div class="reading"><span class="label">Satellite Count</span><span class="value">{liveData.gpsSats}</span></div>
                </section>
                {/if}

                <!-- MPU6050 -->
                <!-- {#if activeSensors.imu}
                <section class="card">
                    <h2>IMU Orientation Sensor (MPU6050)</h2>
                    <div class="reading"><span class="label">Pitch</span><span class="value">{liveData.imuPitch}°</span></div>
                    <div class="reading"><span class="label">Roll</span><span class="value">{liveData.imuRoll}°</span></div>
                </section>
                {/if} -->
                
                <!-- <section class="card full-width admin-panel">
                    <h2>System Administration</h2>

                    <div class="admin-grid">
                        <div class="firmware-section">
                            <h3>Firmware</h3>
                            <p>Pull latest firmware from the local server.</p>
                            <button onclick={handleUpdate} disabled={isUpdating}>
                                {isUpdating ? 'Downloading...' : 'Pull Update'}
                            </button>
                        </div>

                        <div class="wifi-section">
                            <h3>Network Configuration</h3>
                            <p>Update network credentials. Device will reboot upon saving.</p>
                            <input type="text" placeholder="New SSID" bind:value={wifiSSID} />
                            <input type="password" placeholder="New Password" bind:value={wifiPass} />
                            <button onclick={handleWifiUpdate}>Save & Reboot</button>
                            {#if wifiStatusMessage}
                                <p class="status-msg">{wifiStatusMessage}</p>
                            {/if}
                        </div>
                    </div>
                </section> -->
            </div>
        {/if}

        {#if currentPage === 'files'}
            <div class="files-view">
                <header class="page-header">
                    <h2>SD Card Filesystem</h2>
                </header>
                <div class="card full-width">
                    <table class="file-table">
                        <thead>
                            <tr>
                                <th>Filename</th>
                                <th>Size</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            {#each fileList as file}
                            <tr>
                                <td>{file.name}</td>
                                <td>{file.size}</td>
                                <!-- Points to a hypothetical ESP32 endpoint -->
                                <td><a href="/api/download?file={file.name}" download class="btn-download">Download</a></td>
                            </tr>
                            {/each}
                        </tbody>
                    </table>
                </div>
            </div>
        {/if}

        {#if currentPage === 'console'}
            <div class="console-view">
                <div class="terminal" bind:this={terminalContainer}>
                    <pre>{sessionData.terminalLogs}</pre>
                </div>
            </div>
        {/if}
    </main>
<!-- </body> -->

<style>
    /* CSS Variables for Light/Dark Mode */
    :global(:root) {
        --bg-color: #f4f4f5;
        --text-color: #18181b;
        --card-bg: #ffffff;
        --border-color: #e4e4e7;
        --nav-bg: #2563eb;
        --nav-text: #ffffff;
        --term-bg: #1e1e1e;
        --term-text: #4ade80;
    }

    :global(body.dark) {
        --bg-color: #121212;
        --text-color: #e0e0e0;
        --card-bg: #1e1e1e;
        --border-color: #333333;
        --nav-bg: #1f2937;
        --nav-text: #e0e0e0;
        --term-bg: #000000;
        --term-text: #4ade80;
    }

    :global(body) {
        font-family: system-ui, -apple-system, sans-serif;
        background-color: var(--bg-color);
        color: var(--text-color);
        margin: 0;
        padding: 0;
        transition: background-color 0.3s, color 0.3s;
    }

    .app-container {
        max-width: 1000px;
        margin: 0 auto;
        padding: 20px;
    }

    .navbar {
        display: flex;
        justify-content: space-between;
        align-items: center;
        background-color: var(--nav-bg);
        color: var(--nav-text);
        padding: 15px 20px;
        border-radius: 8px;
        margin-bottom: 20px;
    }

    /* .nav-brand {
        font-weight: bold;
        font-size: 1.2rem;
    } */

    .nav-links button {
        background: transparent;
        border: 1px solid transparent;
        color: var(--nav-text);
        padding: 8px 12px;
        cursor: pointer;
        border-radius: 4px;
        font-size: 1rem;
    }

    .nav-links button:hover {
        background: rgba(255, 255, 255, 0.1);
    }

    .nav-links button.active {
        border: 1px solid var(--nav-text);
    }

    .grid-container {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
        gap: 20px;
    }

    .card {
        background: var(--card-bg);
        padding: 20px;
        border-radius: 8px;
        border: 1px solid var(--border-color);
        box-shadow: 0 1px 3px rgba(0,0,0,0.1);
    }

    h2 {
        margin-top: 0;
        font-size: 1.2rem;
        border-bottom: 1px solid var(--border-color);
        padding-bottom: 10px;
    }

    .reading {
        display: flex;
        justify-content: space-between;
        padding: 10px 0;
        border-bottom: 1px solid var(--border-color);
    }

    .reading:last-child {
        border-bottom: none;
    }

    .reading .value {
        font-weight: bold;
    }

    /* Filesystem Table */
    .file-table {
        width: 100%;
        border-collapse: collapse;
        text-align: left;
    }

    .file-table th, .file-table td {
        padding: 12px;
        border-bottom: 1px solid var(--border-color);
    }

    .btn-download {
        background: #2563eb;
        color: white;
        padding: 6px 12px;
        text-decoration: none;
        border-radius: 4px;
        font-size: 0.9rem;
    }

    /* Console View */
    .system-stats {
        display: flex;
        justify-content: space-between;
        margin-bottom: 20px;
        font-weight: bold;
    }

    .terminal {
        background: var(--term-bg);
        color: var(--term-text);
        padding: 20px;
        border-radius: 8px;
        height: 60vh;
        overflow-y: auto;
        font-family: monospace;
        box-shadow: inset 0 0 10px rgba(0,0,0,0.5);
    }

    pre {
        margin: 0;
        white-space: pre-wrap;
    }
</style>