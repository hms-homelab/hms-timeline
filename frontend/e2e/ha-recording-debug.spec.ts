import { test, expect } from '@playwright/test';

test.describe('HA Add-on Recording Debug', () => {

  test('test both cameras playback on local service (same binary as HA)', async ({ page }) => {
    page.setDefaultTimeout(15000);

    await page.goto('http://localhost:8080');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    // --- PATIO (default camera) ---
    console.log('\n=== PATIO CAMERA ===');
    const patioEvent = page.locator('.event-card.cursor-pointer').first();
    const patioCount = await patioEvent.count();
    console.log('Patio events found:', patioCount);

    if (patioCount > 0) {
      const videoReqPromise = page.waitForResponse(
        resp => resp.url().includes('/events/') && resp.url().endsWith('.mp4'),
        { timeout: 10000 }
      );

      await patioEvent.click();

      const videoResp = await videoReqPromise.catch(e => { console.log('No video request caught:', e.message); return null; });
      if (videoResp) {
        console.log('PATIO video URL:', videoResp.url());
        console.log('PATIO status:', videoResp.status());
        console.log('PATIO content-type:', videoResp.headers()['content-type']);
        console.log('PATIO content-length:', videoResp.headers()['content-length']);
      }

      await page.waitForTimeout(2000);

      const video = page.locator('video');
      const videoVisible = await video.isVisible().catch(() => false);
      console.log('PATIO video visible:', videoVisible);

      if (videoVisible) {
        const src = await video.getAttribute('src');
        console.log('PATIO video src:', src);
      }

      const state = await page.evaluate(() => {
        const v = document.querySelector('video');
        if (!v) return { found: false };
        return {
          found: true,
          src: v.src,
          readyState: v.readyState,
          networkState: v.networkState,
          error: v.error ? { code: v.error.code, message: v.error.message } : null,
          duration: v.duration,
          videoWidth: v.videoWidth,
          videoHeight: v.videoHeight,
        };
      });
      console.log('PATIO video state:', JSON.stringify(state, null, 2));

      const notAvail = await page.locator(':text("Recording Not Available")').isVisible().catch(() => false);
      console.log('PATIO "Recording Not Available":', notAvail);

      await page.screenshot({ path: '/tmp/patio-playback.png' });

      // Back to live
      const backBtn = page.locator('button:text("Back to Live")').first();
      if (await backBtn.isVisible()) await backBtn.click();
      await page.waitForTimeout(500);
    }

    // --- FRONT DOOR ---
    console.log('\n=== FRONT DOOR CAMERA ===');
    // Find and click the front_door tab
    const allButtons = page.locator('app-camera-tabs button');
    const buttonCount = await allButtons.count();
    for (let i = 0; i < buttonCount; i++) {
      const text = await allButtons.nth(i).textContent();
      console.log(`Tab ${i}: "${text?.trim()}"`);
    }

    const fdTab = page.locator('app-camera-tabs button').filter({ hasText: /front.?door/i }).first();
    if (await fdTab.count() > 0) {
      await fdTab.click();
      await page.waitForTimeout(1500);

      const fdEvent = page.locator('.event-card.cursor-pointer').first();
      const fdCount = await fdEvent.count();
      console.log('Front door events found:', fdCount);

      if (fdCount > 0) {
        const videoReqPromise2 = page.waitForResponse(
          resp => resp.url().includes('/events/') && resp.url().endsWith('.mp4'),
          { timeout: 10000 }
        );

        await fdEvent.click();

        const videoResp2 = await videoReqPromise2.catch(e => { console.log('No video request caught:', e.message); return null; });
        if (videoResp2) {
          console.log('FRONT_DOOR video URL:', videoResp2.url());
          console.log('FRONT_DOOR status:', videoResp2.status());
          console.log('FRONT_DOOR content-type:', videoResp2.headers()['content-type']);
          console.log('FRONT_DOOR content-length:', videoResp2.headers()['content-length']);
        }

        await page.waitForTimeout(2000);

        const video2 = page.locator('video');
        const videoVisible2 = await video2.isVisible().catch(() => false);
        console.log('FRONT_DOOR video visible:', videoVisible2);

        if (videoVisible2) {
          const src2 = await video2.getAttribute('src');
          console.log('FRONT_DOOR video src:', src2);
        }

        const state2 = await page.evaluate(() => {
          const v = document.querySelector('video');
          if (!v) return { found: false };
          return {
            found: true,
            src: v.src,
            readyState: v.readyState,
            networkState: v.networkState,
            error: v.error ? { code: v.error.code, message: v.error.message } : null,
            duration: v.duration,
            videoWidth: v.videoWidth,
            videoHeight: v.videoHeight,
          };
        });
        console.log('FRONT_DOOR video state:', JSON.stringify(state2, null, 2));

        const notAvail2 = await page.locator(':text("Recording Not Available")').isVisible().catch(() => false);
        console.log('FRONT_DOOR "Recording Not Available":', notAvail2);

        await page.screenshot({ path: '/tmp/frontdoor-playback.png' });
      }
    }
  });
});
