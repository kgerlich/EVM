import puppeteer from 'puppeteer';
import fs from 'fs';
import path from 'path';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  let consoleLines = [];
  let errors = [];
  
  page.on('console', msg => {
    const text = msg.text();
    const type = msg.type();
    console.log(`[Console ${type}]`, text);
    consoleLines.push(text);
    if (type === 'error') {
      errors.push(text);
    }
  });

  page.on('error', err => {
    console.error('[Page Error]', err);
    errors.push(err.message);
  });

  try {
    console.log('Navigating to simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('Waiting for simulator initialization...');
    const initialized = await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 10000 }).catch(() => false);

    if (!initialized) {
      console.log('Module initialization timeout or failed');
    } else {
      console.log('Module initialized successfully!');
    }

    // Wait a bit for WASM to fully load
    await new Promise(r => setTimeout(r, 1000));

    // Check if we can access the simulator worker
    console.log('Testing if simulator worker is accessible...');
    const workerReady = await page.evaluate(() => {
      return new Promise((resolve) => {
        // Send a test message to the worker
        const tempListener = (event) => {
          console.log('[Page] Got response from worker:', event.data);
          window.removeEventListener('message', tempListener);
          resolve(true);
        };
        window.addEventListener('message', tempListener);
        
        // Simulate sending init message (would normally come from React)
        if (window.simulatorWorker) {
          console.log('[Page] Simulator worker exists, sending test message');
          window.simulatorWorker.postMessage({ type: 'test' });
        } else {
          console.log('[Page] Simulator worker not found');
          resolve(false);
        }
      });
    }).catch(() => false);

    if (!workerReady) {
      console.log('⚠️ Could not verify worker readiness');
    }

    // Now test the STEP button directly via the React component
    console.log('Attempting to click STEP button...');
    
    // Wait for the control panel to render
    await page.waitForSelector('[data-testid="step-button"]', { timeout: 5000 }).catch(() => null);
    
    const stepButton = await page.$('[data-testid="step-button"]');
    if (stepButton) {
      console.log('✅ STEP button found!');
      
      // Click it
      await stepButton.click();
      console.log('STEP button clicked');
      
      // Wait a bit for the response
      await new Promise(r => setTimeout(r, 500));
    } else {
      console.log('⚠️ STEP button not found with test ID, trying class selector...');
      
      // Try alternate selector
      const buttons = await page.$$('button');
      for (const btn of buttons) {
        const text = await page.evaluate(el => el.textContent, btn);
        if (text.includes('Step')) {
          console.log('Found Step button via text search');
          await btn.click();
          console.log('Step button clicked');
          await new Promise(r => setTimeout(r, 500));
          break;
        }
      }
    }

    // Check for errors in console
    console.log('\n=== Test Results ===');
    const unreachableErrors = consoleLines.filter(l => l.includes('unreachable'));
    const getValueErrors = consoleLines.filter(l => l.includes('getValue'));
    const memoryErrors = consoleLines.filter(l => l.includes('memory') || l.includes('Cannot read'));

    if (unreachableErrors.length > 0) {
      console.error('❌ UNREACHABLE errors detected:');
      unreachableErrors.forEach(e => console.error('   ', e));
      process.exit(1);
    }

    if (getValueErrors.length > 0) {
      console.error('❌ getValue errors detected:');
      getValueErrors.forEach(e => console.error('   ', e));
      process.exit(1);
    }

    if (memoryErrors.length > 0) {
      console.error('❌ Memory access errors detected:');
      memoryErrors.forEach(e => console.error('   ', e));
      process.exit(1);
    }

    if (errors.length > 0) {
      console.error('❌ Errors detected:');
      errors.forEach(e => console.error('   ', e));
      process.exit(1);
    }

    // Look for success indicators
    const stepExecuted = consoleLines.some(l => l.includes('STEP') || l.includes('Step'));
    const cpuState = consoleLines.some(l => l.includes('CPUState') || l.includes('CPU state') || l.includes('pc:'));

    if (stepExecuted) {
      console.log('✅ STEP execution initiated');
    }
    
    if (cpuState) {
      console.log('✅ CPU state retrieved successfully');
    }

    if (!unreachableErrors.length && !getValueErrors.length && !memoryErrors.length && !errors.length) {
      console.log('✅ Test passed - No errors detected!');
    } else {
      console.log('⚠️ Test completed with warnings');
    }

  } catch (e) {
    console.error('Test error:', e.message);
    console.error('Stack:', e.stack);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
