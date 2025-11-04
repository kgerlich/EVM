import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  page.on('console', msg => {
    const type = msg.type();
    const text = msg.text();
    if (type === 'error') {
      console.log(`[ERROR] ${text}`);
    }
  });

  try {
    console.log('Loading simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('Waiting for WASM...');
    await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 10000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 1000));

    // Wait for ROM to load
    console.log('Waiting for ROM...');
    await page.waitForFunction(() => {
      return document.body.innerText && document.body.innerText.includes('Total');
    }, { timeout: 20000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 2000));

    // Get initial register state
    console.log('\n📋 Initial CPU State (before any instructions):');
    const initialState = await page.evaluate(() => {
      if (!window.__simulatorState) return 'No state available';
      return {
        pc: '0x' + window.__simulatorState.pc.toString(16).padStart(6, '0'),
        sr: '0x' + window.__simulatorState.sr.toString(16).padStart(4, '0'),
        a7: '0x' + window.__simulatorState.aregs[7].toString(16).padStart(8, '0'),
        d0: '0x' + window.__simulatorState.dregs[0].toString(16).padStart(8, '0')
      };
    });
    console.log(JSON.stringify(initialState, null, 2));

    // Click STEP once
    console.log('\n▶️ Executing first instruction (STEP 1)...');
    const buttons = await page.$$('button');
    for (const btn of buttons) {
      const text = await page.evaluate(el => el.textContent, btn);
      if (text.includes('Step')) {
        await btn.click();
        break;
      }
    }

    await new Promise(r => setTimeout(r, 500));

    // Get state after one instruction
    console.log('\n📋 CPU State after STEP 1:');
    const afterStep = await page.evaluate(() => {
      if (!window.__simulatorState) return 'No state available';
      return {
        pc: '0x' + window.__simulatorState.pc.toString(16).padStart(6, '0'),
        sr: '0x' + window.__simulatorState.sr.toString(16).padStart(4, '0'),
        a7: '0x' + window.__simulatorState.aregs[7].toString(16).padStart(8, '0'),
        d0: '0x' + window.__simulatorState.dregs[0].toString(16).padStart(8, '0')
      };
    });
    console.log(JSON.stringify(afterStep, null, 2));

    console.log('\n✅ Test complete');

  } catch (e) {
    console.error('Test error:', e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
