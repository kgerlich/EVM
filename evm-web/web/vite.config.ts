import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

const wasmPlugin = {
  name: 'wasm-mime',
  configureServer(server: any) {
    // Add WASM MIME type middleware BEFORE other middlewares
    server.middlewares.use((req: any, res: any, next: any) => {
      if (req.url && req.url.endsWith('.wasm')) {
        res.setHeader('Content-Type', 'application/wasm');
        console.log('[Vite] Setting WASM MIME type for:', req.url);
      }
      next();
    });
  },
};

export default defineConfig({
  plugins: [wasmPlugin, react()],
  server: {
    headers: {
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    },
  },
  build: {
    target: 'es2020',
    minify: 'terser',
    sourcemap: false,
  },
  optimizeDeps: {
    exclude: ['evm'],
  },
  // Treat WASM files as assets
  assetsInclude: ['**/*.wasm', '**/*.js.wasm'],
})
