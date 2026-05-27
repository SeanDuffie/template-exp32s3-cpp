<script lang="ts">
    import { onMount, tick } from 'svelte';

    // -- App State --
    let currentPage = $state('home'); // 'home', 'console', 'files'
    let isDarkMode = $state(true);
    let terminalContainer: HTMLElement | undefined = $state<HTMLElement>();

    // Handle theme swapping logic
    function toggleTheme() {
        isDarkMode = !isDarkMode;
        if (isDarkMode) {
            document.body.classList.add('dark');
        } else {
            document.body.classList.remove('dark');
        }
    }

    // -- Filesystem Data --
    // Define the shape of the data coming from the ESP32
    interface FileData {
        // name: string;
        size: number;
    }

    // Defines the exact JSON structure sent by your ESP32
    interface StorageAPIResponse {
        totalBytes?: number;
        usedBytes?: number;
        bytesPerRow?: number;
        intervalMinutes?: number;
        files?: { [filename: string]: FileData };
    }

    // Defines your Svelte UI state
    interface StorageState {
        totalKB: number;
        usedKB: number;
        freeKB: number;
        percentUsed: number;
        estimatedDaysLeft: number;
        files: { [filename: string]: FileData };
        error: string | null;
    }

    // Stores file and storage info for LittleFS
    let littleFS = $state<StorageState>({
        totalKB: 0,
        usedKB: 0,
        freeKB: 0,
        percentUsed: 0,
        estimatedDaysLeft: 0,
        files: {} as Record<string, FileData>,
        error: null
    });

    // Store file and storage info for SD Card
    let sdFS = $state<StorageState>({
        totalKB: 0,
        usedKB: 0,
        freeKB: 0,
        percentUsed: 0,
        estimatedDaysLeft: 0,
        files: {} as Record<string, FileData>,
        error: null
    });


    // Helper function to process storage stats
    function processStorageData(targetState: StorageState, file_data: StorageAPIResponse) {
        const total = file_data.totalBytes || 0;
        const used = file_data.usedBytes || 0;
        const bytesPerRow = file_data.bytesPerRow || 80; // Default to 80 if not provided
        const intervalMinutes = file_data.intervalMinutes || 15; // Default to 15 if not provided
        const free = total - used;

        targetState.totalKB = Number((total / 1024).toFixed(1));
        targetState.usedKB = Number((used / 1024).toFixed(1));
        targetState.freeKB = Number((free / 1024).toFixed(1));
        targetState.percentUsed = total > 0 ? Math.round((used / total) * 100) : 0;

        // Estimate Days Left (assuming 80 bytes/row, 15min interval = ~7.68 KB/day)
        const bytesPerDay = bytesPerRow * (60 / intervalMinutes) * 24;
        targetState.estimatedDaysLeft = Math.floor(free / bytesPerDay);
        targetState.files = file_data.files || {};
    }

    async function loadFiles() {
        try {
            // Fetch LittleFS
            const resLFS = await fetch('/api/files?drive=lfs');
            if (resLFS.ok) { processStorageData(littleFS, await resLFS.json()); }
            else {
                littleFS.error = "LittleFS Unavailable. Device may be malfunctioning.";
                console.error("Failed to fetch files from LittleFS.");
                littleFS.totalKB = 0;
                littleFS.usedKB = 0;
                littleFS.freeKB = 0;
                littleFS.percentUsed = 0;
                littleFS.estimatedDaysLeft = 0;
                littleFS.files = {};
            }

            // Fetch SD Card
            const resSD = await fetch('/api/files?drive=sd');
            if (resSD.ok) {
                sdFS.error = null; // Clear any previous error
                processStorageData(sdFS, await resSD.json());
            }
            else {
                sdFS.error = "SD card Disconnected or Unreadable.";
                console.error("Failed to fetch files from SD card.");
                sdFS.totalKB = 0;
                sdFS.usedKB = 0;
                sdFS.freeKB = 0;
                sdFS.percentUsed = 0;
                sdFS.estimatedDaysLeft = 0;
                sdFS.files = {};
            }
        } catch (e) {
            console.error("Error fetching files");
        }
    }

    async function deleteFile(filename: string, drive: 'littlefs' | 'sd') {
        if (!confirm(`Are you sure you want to permanently delete ${filename}?`)) return;
        
        try {
            const resDel = await fetch(`/api/delete?file=${filename}&drive=${drive}`, { method: 'DELETE' });
            if (resDel.ok) {
                // Reload the files and recalculate storage math
                alert(`${filename} deleted successfully. Reloading file list...`);
                await loadFiles();
            } else {
                alert("Failed to delete file.");
            }
        } catch (e) {
            alert("Error during file deletion.");
            console.error("Network error during deletion", e);
        }
    }

    let filePollInterval: number;

    $effect(() => {
        if (currentPage === 'files') {
            // Load immediately upon entering the page
            loadFiles(); 
            // Then poll every 15 seconds (adjust as needed)
            filePollInterval = setInterval(loadFiles, 15000);
        } else {
            // Stop polling when the user navigates away
            clearInterval(filePollInterval);
        }
        
        // Cleanup function for when the component unmounts
        return () => clearInterval(filePollInterval); 
    });

    // Replace DOM contents with new page
    function navigate(page: string) {
        currentPage = page;
        if (page === 'files') {
            loadFiles();
        }
    }

    // // Auto-scroll terminal when new logs arrive
    // $effect(() => {
    //     // This runs automatically whenever currentPage or terminalContainer changes
    //     if (currentPage === 'console' && terminalContainer) {
    //         terminalContainer.scrollTop = terminalContainer.scrollHeight;
    //     }
    // });

    // // System States
    // let isUpdating = $state(false);
    // let wifiSSID = $state("");
    // let wifiPass = $state("");
    // let wifiStatusMessage = $state("");

    // async function handleUpdate() {
    //     isUpdating = true;
    //     try {
    //         // Replace with actual ESP32 API endpoint
    //         const resOTA = await fetch('/api/update', { method: 'POST' });
    //         if (resOTA.ok) alert("Update triggered. Rebooting.");
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
    //         const resWifi = await fetch('/api/wifi', {
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
        lux: false, // BH1750
        soilMoist: false, // Analog Capacitive Soil Moisture Sensor
        soilTemp: false, // DS18B20
        air: false, // BME280
        imu: false, // MPU6050
        gps: false // NEO-6M
    });

    // -- Sensor Data --
    let lightData = $state({
        lux: "",  // Lumen
    });
    let soilData = $state({
        tempF: "", // °F
        tempC: "", // °C
        moisture: "",  // %
        moistureADC: "", // ADC
    })
    let airData = $state({
        tempF: "", // °F
        tempC: "", // °C
        humidity: "",  // %
        pressure: "",  // hPa
    });
    let imuData = $state({
        accelX: "",
        accelY: "",
        accelZ: "",
        gyroX: "",
        gyroY: "",
        gyroZ: "",
        tempF: "",
    });
    let gpsData = $state({
        time: "",  // GPS Timestamp
        lat: "",
        lon: "",
        alt: "",
        speed: "",
        numSats: "",
    });

    onMount(() => {
        // Detect system preference
        if (window.matchMedia && window.matchMedia('(prefers-color-scheme: light)').matches) {
            isDarkMode = false;
        }

        // Apply theme class
        if (isDarkMode) document.body.classList.add('dark');
        else document.body.classList.remove('dark');

        // Establish connection to ESP32
        const socket = new WebSocket(`ws://${window.location.hostname}/ws`);

        socket.onmessage = async (event) => {
            try {
                const data = JSON.parse(event.data);

                // Map JSON keys to your UI variables
                if (data.timestamp) sessionData.systemTime = data.timestamp;
                if (data.uptime) {
                    sessionData.upsec = Number(data.uptime);
                    sessionData.uptime = new Date(sessionData.upsec * 1000).toISOString().slice(11, 19);
                }

                if (data.active) {
                    activeSensors.lux = data.active.lux;
                    activeSensors.soilMoist = data.active.soilMoist;
                    activeSensors.soilTemp = data.active.soilTemp;
                    activeSensors.air = data.active.air;
                    activeSensors.imu = data.active.imu;
                    activeSensors.gps = data.active.gps;
                }

                if (activeSensors.lux && data.lux) {
                    lightData.lux = data.lux.lux || "";
                }

                if (activeSensors.soilMoist && data.soil) {
                    soilData.moisture = data.soil.soilMoisture?.toFixed(2) || "";
                    soilData.moistureADC = data.soil.binMoistRaw || "";
                }

                if (activeSensors.soilTemp && data.soil) {
                    soilData.tempF = data.soil.probeTempF?.toFixed(2) || "";
                    soilData.tempC = data.soil.probeTempC?.toFixed(2) || "";
                }

                if (activeSensors.air && data.air) {
                    airData.tempF = data.air.airTempF?.toFixed(2) || "";
                    airData.tempC = data.air.airTempC?.toFixed(2) || "";
                    airData.humidity = data.air.airHumidity?.toFixed(2) || "";
                    airData.pressure = data.air.airPress?.toFixed(2) || "";
                }

                if (activeSensors.imu && data.imu) {
                    imuData.accelX = data.imu.accelX || "";
                    imuData.accelY = data.imu.accelY || "";
                    imuData.accelZ = data.imu.accelZ || "";
                    imuData.gyroX = data.imu.gyroX || "";
                    imuData.gyroY = data.imu.gyroY || "";
                    imuData.gyroZ = data.imu.gyroZ || "";
                    imuData.tempF = data.imu.tempF || "";
                }

                if (activeSensors.gps && data.gps) {
                    gpsData.time = data.gps.time || "";
                    gpsData.lat = data.gps.latitude || "";
                    gpsData.lon = data.gps.longitude || "";
                    gpsData.alt = data.gps.altitude || "";
                    gpsData.speed = data.gps.speed || "";
                    gpsData.numSats = data.gps.satellites || "";
                }

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
        };
        return () => socket.close();
    });

</script>

<main class="app-container">
    <header>
        <h1>Sensor Node Dashboard</h1>
        <span class="status-indicator">●</span>
    </header>

    <nav class="navbar">
        <div class="nav-links">
            <button class:active={currentPage === 'home'} onclick={() => navigate('home')}>Dashboard</button>
            <button class:active={currentPage === 'files'} onclick={() => navigate('files')}>Filesystem</button>
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
            <!-- Lux (BH1750) -->
            {#if activeSensors.lux}
            <section class="card">
                <h2>Lux Sensor (BH1750)</h2>
                <div class="reading"><span class="label">Light Intensity</span><span class="value">{lightData.lux} Lux</span></div>
            </section>
            {/if}

            <!-- Soil Moisture (Capacitive ADC) -->
            {#if activeSensors.soilMoist}
            <section class="card">
                <h2>Capacitive Soil Moisture Sensor (Analog)</h2>
                <div class="reading"><span class="label">Moisture Level</span><span class="value">{soilData.moisture}%</span></div>
                <div class="reading"><span class="label">Raw ADC</span><span class="value">{soilData.moistureADC}</span></div>
            </section>
            {/if}

            <!-- Soil Temperature (DS18B20) -->
            {#if activeSensors.soilTemp}
            <section class="card">
                <h2>Temperature Probe (DS18B20)</h2>
                <div class="reading"><span class="label">Temperature</span><span class="value">{soilData.tempF}°F</span></div>
                <div class="reading"><span class="label">Temperature</span><span class="value">{soilData.tempF}°F</span></div>
            </section>
            {/if}

            <!-- Air (BME280) -->
            {#if activeSensors.air}
            <section class="card">
                <h2>Air Quality Sensor (BME280)</h2>
                <div class="reading"><span class="label">Air Temp</span><span class="value">{airData.tempF}°F</span></div>
                <div class="reading"><span class="label">Air Temp</span><span class="value">{airData.tempC}°C</span></div>
                <div class="reading"><span class="label">Humidity</span><span class="value">{airData.humidity}%</span></div>
                <div class="reading"><span class="label">Pressure</span><span class="value">{airData.pressure} hPa</span></div>
            </section>
            {/if}

            <!-- IMU (MPU6050) -->
            {#if activeSensors.imu}
            <section class="card">
                <h2>IMU Orientation Sensor (MPU6050)</h2>
                <div class="reading"><span class="label">Accelerometer X</span><span class="value">{imuData.accelX}°</span></div>
                <div class="reading"><span class="label">Accelerometer Y</span><span class="value">{imuData.accelY}°</span></div>
                <div class="reading"><span class="label">Accelerometer Z</span><span class="value">{imuData.accelZ}°</span></div>
                <div class="reading"><span class="label">Gyroscope X</span><span class="value">{imuData.gyroX}°</span></div>
                <div class="reading"><span class="label">Gyroscope Y</span><span class="value">{imuData.gyroY}°</span></div>
                <div class="reading"><span class="label">Gyroscope Z</span><span class="value">{imuData.gyroZ}°</span></div>
                <div class="reading"><span class="label">IMU Temperature</span><span class="value">{imuData.tempF}°F</span></div>
            </section>
            {/if}

            <!-- GPS (NEO-6M) -->
            {#if activeSensors.gps}
            <section class="card">
                <h2>GPS Location Module (NEO-6M)</h2>
                <div class="reading"><span class="label">GPS Time</span><span class="value">{gpsData.time}</span></div>
                <div class="reading"><span class="label">Latitude</span><span class="value">{gpsData.lat}</span></div>
                <div class="reading"><span class="label">Longitude</span><span class="value">{gpsData.lon}</span></div>
                <div class="reading"><span class="label">Altitude</span><span class="value">{gpsData.alt}</span></div>
                <div class="reading"><span class="label">Speed</span><span class="value">{gpsData.speed}</span></div>
                <div class="reading"><span class="label">Satellite Count</span><span class="value">{gpsData.numSats}</span></div>
            </section>
            {/if}
            
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
            <!-- <header class="page-header"> -->
                <h3>Onboard Filesystem</h3>
            <!-- </header> -->
            <div class="card full-width">
                {#if littleFS.error}
                    <div style="color: #ef4444; font-weight: bold; padding: 20px; text-align: center; border: 1px dashed #ef4444; border-radius: 4px; background: rgba(239, 68, 68, 0.1);">
                        ⚠ {littleFS.error}
                    </div>
                {:else}
                    <div>
                        <div class="stat-row">
                            <span class="label">Space Used:</span> {littleFS.usedKB} / {littleFS.totalKB} KB ({littleFS.percentUsed}%)
                        </div>
                        <div class="stat-estimate" style="color: var(--term-text); font-weight: bold;">
                            Estimated logging time remaining: {littleFS.estimatedDaysLeft} days
                        </div>
                    </div>
                    <table class="file-table">
                        <thead>
                            <tr>
                                <th>Filename</th>
                                <th>Size (Bytes)</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            {#each Object.entries(littleFS.files) as [lfs_filename, lfs_data]}
                            <tr>
                                <td>{lfs_filename}</td>
                                <td>{lfs_data.size.toLocaleString()}</td>
                                <!-- Points to a hypothetical ESP32 endpoint -->
                                <td>
                                    <a href="/api/download?file={lfs_filename}&drive=littlefs" download class="btn-download">Download</a>
                                    <button onclick={() => deleteFile(lfs_filename, 'littlefs')} class="btn-delete">Purge</button>
                                </td>
                            </tr>
                            {/each}
                        </tbody>
                    </table>
                {/if}
            </div>
            <p>
                <em>*Note: Purging the index.html file or any icons will break the web interface.*</em>
            </p>

            <!-- <header class="page-header"> -->
                <h3>SD Card Filesystem</h3>
            <!-- </header> -->
            <div class="card full-width">
                {#if sdFS.error}
                    <div style="color: #ef4444; font-weight: bold; padding: 20px; text-align: center; border: 1px dashed #ef4444; border-radius: 4px; background: rgba(239, 68, 68, 0.1);">
                        ⚠ {sdFS.error}
                    </div>
                {:else}
                    <div>
                        <div class="stat-row">
                            <span class="label">Space Used:</span> {sdFS.usedKB} / {sdFS.totalKB} KB ({sdFS.percentUsed}%)
                        </div>
                        <div class="stat-estimate" style="color: var(--term-text); font-weight: bold;">
                            Estimated logging time remaining: {sdFS.estimatedDaysLeft} days
                        </div>
                    </div>
                    <table class="file-table">
                        <thead>
                            <tr>
                                <th>Filename</th>
                                <th>Size (Bytes)</th>
                                <th>Action</th>
                            </tr>
                        </thead>
                        <tbody>
                            {#each Object.entries(sdFS.files) as [sd_filename, sd_data]}
                            <tr>
                                <td>{sd_filename}</td>
                                <td>{sd_data.size.toLocaleString()}</td>
                                <!-- Points to a hypothetical ESP32 endpoint -->
                                <td>
                                    <a href="/api/download?file={sd_filename}&drive=sd" download class="btn-download">Download</a>
                                    <button onclick={() => deleteFile(sd_filename, 'sd')} class="btn-delete">Purge</button>
                                </td>
                            </tr>
                            {/each}
                        </tbody>
                    </table>
                {/if}
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

<style>
    /* CSS Variables for Light/Dark Mode */
    :global(:root) {
        --bg-color: #e2e8f0; /* Darker slate-grey for the background */
        --text-color: #0f172a; /* Deep slate for text */
        --card-bg: #ffffff; /* Pure white for cards to pop */
        --border-color: #cbd5e1;
        --title-color: #0f172a; /* Distinct color for titles */
        --heading-color: #1d4ed8; /* Distinct blue for headings */
        --nav-bg: #2563eb;
        --nav-text: #ffffff;
        --term-bg: #1e1e1e;
        --term-text: #4ade80;
    }

    :global(body.dark) {
        --bg-color: #0f172a; /* Dark Slate */
        --text-color: #f8fafc; /* Off-White/Ice */
        --card-bg: #1e293b; /* Elevated Slate */
        --border-color: #334155; /* Mid Slate */
        --title-color: #e2e8f0; /* Distinct color for titles */
        --heading-color: #1d4ed8; /* Distinct blue for headings */
        --nav-bg: #1e40af; /* Deep Blue */
        --nav-text: #f8fafc;
        --term-bg: #020617; /* Near Black */
        --term-text: #4ade80; /* Phosphor Green */
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

    h1 {
        color: var(--title-color); /* Apply the title color */
    }

    h2 {
        margin-top: 0;
        font-size: 1.2rem;
        color: var(--heading-color); /* Apply the heading color */
        border-bottom: 2px solid var(--border-color); /* Thicker border */
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

    .btn-delete {
        background: #dc2626;
        color: white;
        border: none;
        padding: 6px 12px;
        border-radius: 4px;
        cursor: pointer;
        margin-left: 10px;
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