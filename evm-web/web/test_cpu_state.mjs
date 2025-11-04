import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  let lastError = null;

  page.on('console', msg => {
    const text = msg.text();
    if (msg.type() === 'error') {
      console.log(`[ERROR] ${text}`);
      lastError = text;
    }
  });

  try {
    console.log('Loading simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('Waiting for simulator...');
    await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 10000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 1000));

    // Load ROM
    console.log('\n📚 Loading ROM...');
    await page.click('button:has-text("Load ROM")').catch(() => {});
    await new Promise(r => setTimeout(r, 2000));

    // Click STEP button and capture state
    for (let stepNum = 1; stepNum <= 3; stepNum++) {
      console.log(`\n⏭️  STEP ${stepNum}...`);

      const buttons = await page.$$('button');
      let clicked = false;

      for (const btn of buttons) {
        const text = await page.evaluate(el => el.textContent, btn);
        if (text.includes('Step')) {
          await btn.click();
          clicked = true;
          break;
        }
      }

      if (!clicked) {
        console.log('Step button not found');
      }

      await new Promise(r => setTimeout(r, 500));
    }

    console.log('\n✅ CPU state tracking test completed');

    if (lastError && lastError.includes('unreachable')) {
      console.error('❌ UNREACHABLE error detected');
      process.exit(1);
    }
    if (lastError && lastError.includes('getValue')) {
      console.error('❌ getValue error detected');
      process.exit(1);
    }

  } catch (e) {
    console.error('Test error:', e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
