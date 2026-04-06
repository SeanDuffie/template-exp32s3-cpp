import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import { viteSingleFile } from 'vite-plugin-singlefile';
import viteCompression from 'vite-plugin-compression'

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    svelte(),
    viteSingleFile(),
    viteCompression({
      algorithm: 'gzip',
      ext: '.gz',
      filter: /\.(html)$/i, // Force it to only look at the final single file
      threshold: 512, // Compress anything larger than 512 bytes
      deleteOriginFile: true // Delete the uncompressed files to save ESP32 flash space
    })
  ],
  base: './', // Forces Vite to use relative paths for CSS and JS assets
  build: {
    // Output directly to the PlatformIO data folder
    outDir: '../data', 
    // Wipe the old UI files before building the new ones
    emptyOutDir: true, 
    minify: 'terser',
    cssCodeSplit: false,rollupOptions: {
            output: {
                manualChunks: undefined,
            }
        }
  }
})
