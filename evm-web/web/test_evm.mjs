import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox', '--disable-setuid-sandbox']
  });

  const page = await browser.newPage();

  // Capture console messages
  const consoleLogs = [];
  page.on('console', msg => {
    const log = {
      type: msg.type(),
      text: msg.text(),
      location: msg.location()
    };
    consoleLogs.push(log);
    console.log(`[${log.type.toUpperCase()}] ${log.text}`);
  });

  // Capture page errors
  page.on('error', err => console.error('Page error:', err));
  page.on('pageerror', err => console.error('Page error:', err));

  console.log('🌐 Opening http://localhost:8084...');
  try {
    await page.goto('http://localhost:8084', { waitUntil: 'networkidle2', timeout: 30000 });
  } catch (e) {
    console.warn('⚠️ Navigation timeout (expected for WASM): ' + e.message);
  }

  console.log('⏳ Waiting for simulator initialization (5 seconds)...');
  await new Promise(r => setTimeout(r, 5000));

  console.log('\n📋 === CHECKING FOR READ-ONLY MEMORY ERRORS ===');
  const hasROMErrors = consoleLogs.some(log =>
    log.text.includes('read-only') || log.text.includes('ROM:') && log.text.includes('Attempted')
  );

  if (hasROMErrors) {
    console.log('❌ FOUND ROM READ-ONLY ERRORS:');
    consoleLogs.filter(log => (log.text.includes('read-only') || log.text.includes('ROM:')) && log.text.includes('Attempted')).forEach(log => {
      console.log(`   [${log.type}] ${log.text}`);
    });
  } else {
    console.log('✅ NO ROM READ-ONLY ERRORS DETECTED');
  }

  console.log('\n📋 === CHECKING FOR SUCCESSFUL ROM LOAD ===');
  const hasROMLoad = consoleLogs.some(log =>
    log.text.includes('LOADROM: Success') || log.text.includes('ROM loaded')
  );

  if (hasROMLoad) {
    console.log('✅ FOUND ROM LOAD SUCCESS:');
    consoleLogs.filter(log => log.text.includes('LOADROM') || log.text.includes('ROM loaded')).forEach(log => {
      console.log(`   [${log.type}] ${log.text}`);
    });
  } else {
    console.log('⚠️ NO EXPLICIT ROM LOAD SUCCESS MESSAGE');
  }

  console.log('\n📋 === WORKER INITIALIZATION SEQUENCE ===');
  const workerLogs = consoleLogs.filter(log => log.text.includes('[Worker]'));
  if (workerLogs.length > 0) {
    workerLogs.slice(0, 30).forEach(log => {
      console.log(`[${log.type}] ${log.text}`);
    });
    if (workerLogs.length > 30) {
      console.log(`... and ${workerLogs.length - 30} more worker messages`);
    }
  } else {
    console.log('(No worker messages found)');
  }

  console.log('\n✅ Test complete');
  await browser.close();
})().catch(err => {
  console.error('Test failed:', err);
  process.exit(1);
});
