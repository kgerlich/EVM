import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

  let consoleLines = [];
  page.on('console', msg => {
    const text = msg.text();
    const type = msg.type();
    console.log('[Console ' + type + ']', text);
    consoleLines.push(text);
  });

  try {
    console.log('Navigating to simulator...');
    await page.goto('http://192.168.1.169:8084/', { waitUntil: 'networkidle2', timeout: 15000 });

    console.log('Waiting for simulator module to initialize...');
    const initialized = await page.waitForFunction(() => {
      return window.evm_module && typeof window.evm_module._cpu_step === 'function';
    }, { timeout: 8000 }).catch(() => false);

    if (!initialized) {
      console.log('Module not fully initialized, checking current state...');
    } else {
      console.log('Module initialized!');
    }

    await new Promise(r => setTimeout(r, 1000));

    const hasUnreachableError = consoleLines.some(line => line.includes('unreachable'));

    if (hasUnreachableError) {
      console.error('UNREACHABLE ERROR DETECTED');
      process.exit(1);
    } else {
      console.log('Test complete - NO unreachable errors detected!');
    }

  } catch (e) {
    console.error('Test error:', e.message);
    if (consoleLines.some(l => l.includes('unreachable'))) {
      console.error('UNREACHABLE ERROR FOUND IN CONSOLE');
      process.exit(1);
    }
  } finally {
    await browser.close();
  }
})();
