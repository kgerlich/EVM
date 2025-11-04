import puppeteer from 'puppeteer';

(async () => {
  const browser = await puppeteer.launch({
    headless: true,
    args: ['--no-sandbox']
  });
  const page = await browser.newPage();

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

    // Read ROM bytes
    console.log('\n📖 Reading ROM from simulator...');

    const romData = await page.evaluate(() => {
      // Access WASM memory directly via the simulator
      if (!window.evm_module) {
        return { error: 'WASM module not available' };
      }

      const Module = window.evm_module;
      if (!Module.getValue) {
        return { error: 'Module.getValue not available' };
      }

      // Read 64 bytes from ROM (0x000000 to 0x00003F)
      const bytes = [];
      for (let i = 0; i < 64; i++) {
        const byte = Module.getValue(i, 'i8') & 0xFF;
        bytes.push(byte);
      }
      return bytes;
    });

    if (romData.error) {
      console.log('⚠️  ' + romData.error);
    } else {
      console.log('\nROM Hex Dump (bytes 0x000000 - 0x00003F):');

      // Print hex with ASCII
      for (let i = 0; i < romData.length; i += 16) {
        const offset = '0x' + i.toString(16).padStart(6, '0');
        const hex = romData.slice(i, i + 16)
          .map(b => b.toString(16).padStart(2, '0').toUpperCase())
          .join(' ');
        const ascii = romData.slice(i, i + 16)
          .map(b => b >= 32 && b < 127 ? String.fromCharCode(b) : '.')
          .join('');
        console.log(`${offset}  ${hex.padEnd(48)} ${ascii}`);
      }

      console.log('\n📋 First Instructions (disassembly guide):');
      console.log(`0x000000: [${romData[0].toString(16).padStart(2, '0')} ${romData[1].toString(16).padStart(2, '0')}] - First opcode word`);
      console.log(`0x000002: [${romData[2].toString(16).padStart(2, '0')} ${romData[3].toString(16).padStart(2, '0')}] - Second opcode word (where PC gets stuck)`);

      // Check for specific patterns
      const firstOpcode = (romData[0] << 8) | romData[1];
      const secondOpcode = (romData[2] << 8) | romData[3];

      console.log(`\nOpcode Analysis:`);
      console.log(`First word: 0x${firstOpcode.toString(16).padStart(4, '0')}`);
      console.log(`Second word: 0x${secondOpcode.toString(16).padStart(4, '0')}`);

      if (secondOpcode === 0x4E72) {
        console.log('\n⚠️  FOUND STOP INSTRUCTION at 0x000002! (0x4E72)');
        console.log('This explains why PC never advances beyond 0x000002');
      } else if (firstOpcode === 0x4E72) {
        console.log('\n⚠️  FOUND STOP INSTRUCTION at 0x000000! (0x4E72)');
      }
    }

  } catch (e) {
    console.error('Test error: ' + e.message);
    process.exit(1);
  } finally {
    await browser.close();
  }
})();
