import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  const pcValues = [];

  page.on('console', msg => {
    const text = msg.text();
    if (text.includes('PC=0x')) {
      const match = text.match(/PC=0x([0-9a-fA-F]+)/);
      if (match) {
        const pc = parseInt(match[1], 16);
        pcValues.push(pc);
      }
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

    console.log('📚 Waiting for ROM...');
    await page.waitForFunction(() => {
      return document.body.innerText && document.body.innerText.includes('Total');
    }, { timeout: 20000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 2000));

    // Find the STEP button using class selector (like test_step.mjs does)
    console.log('\n⏭️  Clicking STEP button 10 times...\n');

    for (let i = 0; i < 10; i++) {
      const stepButton = await page.evaluate(() => {
        const buttons = document.querySelectorAll('button');
        for (const btn of buttons) {
          if (btn.textContent.includes('Step') || btn.className.includes('step')) {
            return true;
          }
        }
        return false;
      });

      if (!stepButton) {
        console.log(`Step ${i+1}: Button not found`);
        break;
      }

      try {
        await page.click('button');  // Click first button for now
        await new Promise(r => setTimeout(r, 500));
        console.log(`Step ${i+1}: Clicked (checking logs for PC...)`);
      } catch (e) {
        console.log(`Step ${i+1}: Click failed - ${e.message}`);
        break;
      }
    }

    // Wait a bit for all console messages
    await new Promise(r => setTimeout(r, 1000));

    console.log('\n📊 PC Values from logs:');
    if (pcValues.length === 0) {
      console.log('⚠️  No PC values captured from console');

      // Try reading from state directly
      const directPC = await page.evaluate(() => {
        return window.__simulatorState?.pc || 'unknown';
      });
      console.log('Direct state read - PC: 0x' + directPC.toString(16).padStart(6, '0'));
    } else {
      console.log('Captured: ' + pcValues.map(v => '0x' + v.toString(16).padStart(6, '0')).join(' → '));

      const uniquePCs = new Set(pcValues);
      console.log(`\nProgression: ${uniquePCs.size} unique values in ${pcValues.length} captures`);

      if (pcValues[pcValues.length-1] === pcValues[pcValues.length-2]) {
        console.log('⚠️  PC appears stuck at last value');
      } else {
        console.log('✅ PC is advancing');
      }
    }

  } catch (e) {
    console.error('Test error: ' + e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
