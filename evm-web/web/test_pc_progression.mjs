import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  const pcValues = [];
  let errorCount = 0;

  page.on('console', msg => {
    const type = msg.type();
    const text = msg.text();
    if (type === 'error' && text.includes('BUS ERROR')) {
      errorCount++;
    }
  });

  try {
    console.log('🚀 Loading simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('⏳ Waiting for WASM...');
    await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 10000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 1000));

    // Wait for ROM to load
    console.log('📚 Loading ROM...');
    await page.waitForFunction(() => {
      return document.body.innerText && document.body.innerText.includes('Total');
    }, { timeout: 20000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 2000));

    // Click STEP 10 times and track PC
    console.log('\n⏭️  Stepping through 10 instructions...\n');
    const buttons = await page.$$('button');
    let stepButton = null;
    for (const btn of buttons) {
      const text = await page.evaluate(el => el.textContent, btn);
      if (text.includes('Step')) {
        stepButton = btn;
        break;
      }
    }

    for (let i = 0; i < 10; i++) {
      // Get PC before step
      const beforePC = await page.evaluate(() => {
        return window.__simulatorState?.pc || 0;
      });

      if (stepButton) {
        try {
          await stepButton.click();
          await new Promise(r => setTimeout(r, 300));
        } catch (e) {
          console.log(`  [${i}] Error clicking STEP: ${e.message}`);
        }
      }

      // Get PC after step
      const afterPC = await page.evaluate(() => {
        return window.__simulatorState?.pc || 0;
      });

      const pcHex = '0x' + afterPC.toString(16).padStart(6, '0');
      pcValues.push(afterPC);

      const changed = beforePC !== afterPC ? 'advanced' : 'STUCK';
      console.log(`  [${i+1}] PC: ${pcHex} (${changed})`);

      if (beforePC === afterPC && i > 1) {
        console.log(`\n⚠️  PC stuck at ${pcHex} after step ${i+1}`);
        break;
      }
    }

    console.log('\n📊 PC Progression Summary:');
    console.log('Values: ' + pcValues.map(v => '0x' + v.toString(16).padStart(6, '0')).join(' → '));
    console.log('Bus errors during steps: ' + errorCount);

    const uniquePCs = new Set(pcValues);
    if (uniquePCs.size === 1) {
      console.log('\n❌ CRITICAL: PC never advanced!');
    } else if (uniquePCs.size < pcValues.length) {
      console.log(`\n⚠️  PC jumped between ${uniquePCs.size} different values`);
    }

  } catch (e) {
    console.error('Test error: ' + e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
