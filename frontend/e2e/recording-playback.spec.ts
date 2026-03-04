import { test, expect } from '@playwright/test';

test.describe('Recording Playback', () => {
  test('app loads and shows camera tabs', async ({ page }) => {
    await page.goto('/');
    // Wait for cameras to load (loading spinner disappears)
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });
  });

  test('events list loads for default camera', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });
    // Wait for either events or "No events" message
    await expect(
      page.locator('app-event-list, :text("No events found")')
    ).toBeVisible({ timeout: 10000 });
  });

  test('clicking event with recording opens video player', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    // Find an event card that has a play icon (SVG with the play path = has detections)
    const eventCard = page.locator('.event-card.cursor-pointer').first();
    const hasPlayableEvent = await eventCard.count() > 0;

    if (!hasPlayableEvent) {
      test.skip(true, 'No playable events available');
      return;
    }

    await eventCard.click();

    // Video element should appear
    const video = page.locator('video');
    await expect(video).toBeVisible({ timeout: 5000 });

    // Check that video has a src attribute set
    const src = await video.getAttribute('src');
    expect(src).toBeTruthy();
    expect(src).toContain('events/');
    console.log('Video src:', src);
  });

  test('video element loads successfully (no error)', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    const eventCard = page.locator('.event-card.cursor-pointer').first();
    const hasPlayableEvent = await eventCard.count() > 0;
    if (!hasPlayableEvent) {
      test.skip(true, 'No playable events available');
      return;
    }

    await eventCard.click();

    const video = page.locator('video');
    await expect(video).toBeVisible({ timeout: 5000 });

    // Wait for loadedmetadata, loadeddata, or error event on the video.
    // Headless Chromium may lack codecs for full decode, so accept loadedmetadata too.
    const result = await page.evaluate(() => {
      return new Promise<{ success: boolean; event?: string; error?: string; src?: string }>((resolve) => {
        const video = document.querySelector('video');
        if (!video) {
          resolve({ success: false, error: 'No video element found' });
          return;
        }
        // If video already has metadata loaded (readyState >= 1), it's already good
        if (video.readyState >= 1) {
          resolve({ success: true, event: 'already-loaded', src: video.src });
          return;
        }
        const timeout = setTimeout(() => {
          resolve({ success: false, error: 'Timeout waiting for video load', src: video.src });
        }, 10000);

        const onSuccess = (eventName: string) => {
          clearTimeout(timeout);
          resolve({ success: true, event: eventName, src: video.src });
        };

        video.addEventListener('loadedmetadata', () => onSuccess('loadedmetadata'), { once: true });
        video.addEventListener('loadeddata', () => onSuccess('loadeddata'), { once: true });

        video.addEventListener('error', () => {
          clearTimeout(timeout);
          const mediaError = video.error;
          resolve({
            success: false,
            error: `MediaError code=${mediaError?.code} message=${mediaError?.message}`,
            src: video.src,
          });
        }, { once: true });
      });
    });

    console.log('Video load result:', JSON.stringify(result, null, 2));
    expect(result.success).toBe(true);
  });

  test('video request returns 200 with correct content-type', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    const eventCard = page.locator('.event-card.cursor-pointer').first();
    const hasPlayableEvent = await eventCard.count() > 0;
    if (!hasPlayableEvent) {
      test.skip(true, 'No playable events available');
      return;
    }

    // Intercept network requests for video files
    const videoRequestPromise = page.waitForResponse(
      (resp) => resp.url().includes('/events/') && resp.url().endsWith('.mp4'),
      { timeout: 15000 }
    );

    await eventCard.click();

    const videoResponse = await videoRequestPromise;
    console.log('Video URL:', videoResponse.url());
    console.log('Video status:', videoResponse.status());
    console.log('Content-Type:', videoResponse.headers()['content-type']);

    expect(videoResponse.status()).toBe(200);
    expect(videoResponse.headers()['content-type']).toContain('video');
  });

  test('"Recording Not Available" overlay shows on error', async ({ page }) => {
    // Navigate to app and manually trigger an invalid recording URL
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    const eventCard = page.locator('.event-card.cursor-pointer').first();
    const hasPlayableEvent = await eventCard.count() > 0;
    if (!hasPlayableEvent) {
      test.skip(true, 'No playable events available');
      return;
    }

    await eventCard.click();
    const video = page.locator('video');
    await expect(video).toBeVisible({ timeout: 5000 });

    // Force an error by changing the video src to a non-existent file
    await page.evaluate(() => {
      const video = document.querySelector('video');
      if (video) video.src = 'events/nonexistent_file.mp4';
    });

    // The "Recording Not Available" overlay should appear
    await expect(page.locator(':text("Recording Not Available")')).toBeVisible({ timeout: 10000 });
  });

  test('back to live button works from recording view', async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });

    const eventCard = page.locator('.event-card.cursor-pointer').first();
    const hasPlayableEvent = await eventCard.count() > 0;
    if (!hasPlayableEvent) {
      test.skip(true, 'No playable events available');
      return;
    }

    await eventCard.click();
    await expect(page.locator('video')).toBeVisible({ timeout: 5000 });

    // Click "Back to Live"
    await page.locator('button:text("Back to Live")').first().click();

    // Video should be hidden, live view should be back
    await expect(page.locator('video')).not.toBeVisible();
    // LIVE badge should be visible
    await expect(page.locator('.bg-red-600')).toBeVisible({ timeout: 5000 });
  });
});
