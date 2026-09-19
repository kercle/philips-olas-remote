async function sendCommand(url) {
    const status =
        document.getElementById('status');
    status.textContent = 'Sending…';

    try {
        const response = await fetch(url, {
            method: 'POST',
            cache: 'no-store'
        });
        const text = await response.text();

        if (response.ok) {
            status.textContent = text;
        } else {
            status.textContent = 'Error: ' + text;
        }
    } catch (e) {
        status.textContent =
            'Connection failed';
    }
}
