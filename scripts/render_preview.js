const puppeteer = require('puppeteer');
const fs = require('fs');
const { execSync } = require('child_process');
const path = require('path');

(async () => {
  const outGif = process.argv[2] || path.join('assets','preview_flow.gif');
  const duration = parseInt(process.argv[3] || '8', 10);
  const fps = parseInt(process.argv[4] || '15', 10);
  const frames = duration * fps;
  const tmpDir = path.join('tmp_preview_frames');
  if (!fs.existsSync(tmpDir)) fs.mkdirSync(tmpDir, { recursive: true });

  const browser = await puppeteer.launch({ args: ['--no-sandbox'] });
  const page = await browser.newPage();
  await page.setViewport({ width: 1000, height: 800 });
  const fileUrl = 'file://' + path.resolve('assets', 'preview_page.html');
  await page.goto(fileUrl);
  await page.waitForTimeout(500); // let animation start

  for (let i = 0; i < frames; i++) {
    const filename = path.join(tmpDir, `frame_${String(i).padStart(4,'0')}.png`);
    await page.screenshot({ path: filename });
    await page.waitForTimeout(Math.round(1000 / fps));
  }

  await browser.close();

  // generate palette + gif via ffmpeg
  try {
    execSync(`ffmpeg -y -framerate ${fps} -i ${tmpDir}/frame_%04d.png -vf palettegen ${tmpDir}/palette.png`, { stdio: 'inherit' });
    execSync(`ffmpeg -y -framerate ${fps} -i ${tmpDir}/frame_%04d.png -i ${tmpDir}/palette.png -lavfi paletteuse ${outGif}`, { stdio: 'inherit' });
    console.log('GIF generated:', outGif);
  } catch (e) {
    console.error('ffmpeg step failed. Ensure ffmpeg is installed and in PATH.');
    process.exit(1);
  }
})();