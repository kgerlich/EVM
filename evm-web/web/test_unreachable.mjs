import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({ 
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  // Capture console messages
  let errors = [];
  page.on('console', msg => {
    if (msg.type() === 'error') {
      errors.push(msg.text());
    }
  });

  try {
    console.log('Navigating to simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 10000 });
    
    console.log('Waiting for simulator initialization...');
    await page.waitForFunction(() => window.simulatorReady, { timeout: 5000 }).catch(() => {});
    
    // Wait a bit for WASM module to fully load
    await new Promise(r => setTimeout(r, 1000));
    
    console.log('Checking for unreachable errors...');
    const pageErrors = await page.evaluate(() => {
      return window.__runtimeErrors || [];
    });
    
    console.log('Testing CPU execution with step...');
    const result = await page.evaluate(() => {
      try {
        // Use the exposed function directly if available
        if (window.evm_module && window.evm_module._cpu_step) {
          const err = window.evm_module._cpu_step();
          return { success: true, error: err };
        }
        return { success: false, error: 'Module not ready' };
      } catch (e) {
        return { success: false, error: e.message || e.toString() };
      }
    });
    
    console.log('Step result:', result);
    
    if (errors.includes('unreachable')) {
      console.error('❌ UNREACHABLE ERROR DETECTED');
      process.exit(1);
    } else if (result.success === false) {
      console.log('⚠️  Execution test result:', result);
    } else {
      console.log('✅ No unreachable errors detected!');
    }
    
  } catch (e) {
    console.error('Test error:', e);
  } finally {
    await browser.close();
  }
})();
