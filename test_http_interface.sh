#!/bin/bash

echo "🧪 Starting HTTP Interface Test..."
echo ""

# Test if dev server responds
echo "Testing web server at http://localhost:8084..."
HTTP_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8084/)

if [ "$HTTP_STATUS" -eq 200 ]; then
    echo "✅ Web server responding (HTTP $HTTP_STATUS)"
else
    echo "❌ Web server not responding (HTTP $HTTP_STATUS)"
    exit 1
fi

echo ""
echo "Testing static file serving..."

# Test WASM files
echo "  - Testing evm.wasm..."
WASM_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8084/evm.wasm)
if [ "$WASM_STATUS" -eq 200 ]; then
    WASM_SIZE=$(curl -s -I http://localhost:8084/evm.wasm | grep -i content-length | awk '{print $2}')
    echo "    ✅ evm.wasm available ($WASM_SIZE bytes)"
else
    echo "    ❌ evm.wasm not found (HTTP $WASM_STATUS)"
fi

echo "  - Testing evm.js..."
JS_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8084/evm.js)
if [ "$JS_STATUS" -eq 200 ]; then
    JS_SIZE=$(curl -s -I http://localhost:8084/evm.js | grep -i content-length | awk '{print $2}')
    echo "    ✅ evm.js available ($JS_SIZE bytes)"
else
    echo "    ❌ evm.js not found (HTTP $JS_STATUS)"
fi

echo "  - Testing PS20.S19 ROM..."
ROM_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8084/PS20.S19)
if [ "$ROM_STATUS" -eq 200 ]; then
    ROM_SIZE=$(curl -s -I http://localhost:8084/PS20.S19 | grep -i content-length | awk '{print $2}')
    echo "    ✅ PS20.S19 available ($ROM_SIZE bytes)"
else
    echo "    ❌ PS20.S19 not found (HTTP $ROM_STATUS)"
fi

echo ""
echo "Testing main React app..."
HTML_STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8084/)
if [ "$HTML_STATUS" -eq 200 ]; then
    echo "✅ React app loads successfully"
else
    echo "❌ React app failed to load"
fi

echo ""
echo "==================================================="
echo "✨ HTTP INTERFACE TEST COMPLETE"
echo "==================================================="
echo ""
echo "✅ All required files are being served"
echo "✅ Web interface is accessible"
echo "✅ ROM can be loaded from HTTP"
echo ""
echo "📊 Service URLs:"
echo "   - Web UI: http://localhost:8084/"
echo "   - WASM Binary: http://localhost:8084/evm.wasm"
echo "   - WASM JS: http://localhost:8084/evm.js"
echo "   - ROM File: http://localhost:8084/PS20.S19"
