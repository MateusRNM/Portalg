import tailwindcss from '@tailwindcss/vite';
import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

export default defineConfig({ 
    plugins: [tailwindcss(), sveltekit()],
    optimizeDeps: {
        exclude: ['interpreter/portalg.js']
    },
    server: {
        port: 5173,
        strictPort: true,
        host: true,
        fs: {
            strict: false
        }
    },
    build: {
        target: 'esnext'
    }
});
