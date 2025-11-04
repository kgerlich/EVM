import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  let consoleOutput = [];
  let busErrors = [];
  let lastPC = null;

  page.on('console', msg => {
    const text = msg.text();
    const type = msg.type();
    consoleOutput.push(text);

    if (type === 'error') {
      console.log(`[ERROR] ${text}`);
      if (text.includes('BUS ERROR')) {
        busErrors.push(text);
      }
    }
  });

  try {
    console.log('🚀 Loading simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('⏳ Waiting for WASM initialization...');
    await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 10000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 1000));

    console.log('\n📚 Loading ROM...');
    // Wait for initial load
    await page.waitForFunction(() => {
      const text = document.body.innerText;
      return text && text.includes('Total');
    }, { timeout: 20000 }).catch(() => false);

    await new Promise(r => setTimeout(r, 2000));

    console.log('\n🎮 Clicking PLAY button...');
    try {
      const buttons = await page.$$('button');
      let playClicked = false;

      for (const btn of buttons) {
        const text = await page.evaluate(el => el.textContent, btn);
        if (text.includes('Play') || text.includes('Run')) {
          console.log(`Found button: "${text}"`);
          await btn.click();
          playClicked = true;
          console.log('✅ PLAY clicked');
          break;
        }
      }

      if (!playClicked) {
        console.log('⚠️ Play button not found, trying alternative selectors');
      }
    } catch (e) {
      console.log('⚠️ Error clicking PLAY:', e.message);
    }

    console.log('\n⏱️ Waiting 3 seconds for execution (monitoring for errors)...');
    // Keep monitoring console for bus errors during execution
    for (let i = 0; i < 3; i++) {
      await new Promise(r => setTimeout(r, 1000));
      console.log(`  [${i+1}s] Bus errors so far: ${busErrors.length}`);
    }

    console.log('\n🛑 Attempting to click PAUSE button...');
    try {
      const pauseButtons = await page.$$('button');
      for (const btn of pauseButtons) {
        const text = await page.evaluate(el => el.textContent, btn);
        if (text.includes('Pause') || text.includes('Stop')) {
          console.log(`Found button: "${text}"`);
          await btn.click();
          console.log('✅ PAUSE clicked');
          break;
        }
      }
    } catch (e) {
      console.log('⚠️ Error clicking PAUSE:', e.message);
    }

    await new Promise(r => setTimeout(r, 500));

    // Get final CPU state
    let finalState = {};
    try {
      finalState = await page.evaluate(() => {
        return window.__simulatorState || {};
      });
    } catch (e) {
      console.log('⚠️ Could not read final CPU state:', e.message);
    }

    console.log('\n📊 === Final Execution Results ===');
    console.log('PC: ' + (finalState.pc || 'unknown'));
    console.log('SR: ' + (finalState.sr || 'unknown'));
    console.log('Bus Errors Encountered: ' + busErrors.length);

    if (busErrors.length > 0) {
      console.log('\n🔍 Bus Error Details (first 10):');
      const uniqueErrors = [...new Set(busErrors)];
      uniqueErrors.slice(0, 10).forEach(err => {
        console.log('  - ' + err);
      });
      if (uniqueErrors.length > 10) {
        console.log('  ... and ' + (uniqueErrors.length - 10) + ' more');
      }
    }

    console.log('\n✅ Test completed!');
    console.log('Total console messages: ' + consoleOutput.length);

  } catch (e) {
    console.error('❌ Test error:', e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
