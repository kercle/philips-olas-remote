const CACHE_NAME = 'pwa-cache-v1';
const ASSETS_TO_CACHE = [
    '/',
    '/index.html',
    '/css/index.css',
    '/regular.min.css',
    '/solid.min.css',
    '/js/index.js',
    '/webfonts/fa-regular-custom.woff2',
    '/webfonts/fa-solid-custom.woff2'
];

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            return cache.addAll(ASSETS_TO_CACHE);
        })
    );
});

self.addEventListener('fetch', (event) => {
    event.respondWith(
        caches.match(event.request).then((response) => {
            return response || fetch(event.request);
        })
    );
});
