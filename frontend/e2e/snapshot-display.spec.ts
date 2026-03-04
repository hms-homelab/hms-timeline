import { test, expect } from '@playwright/test';

test.describe('Periodic Snapshot Display', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });
  });

  async function clickSnapshotDot(page: any) {
    // The 6px snapshot dots are often intercepted by the parent container.
    // Use JavaScript dispatch to ensure Angular's (click) handler fires.
    await page.evaluate(() => {
      const dot = document.querySelector('app-timeline button.bg-gray-500') as HTMLElement;
      if (dot) dot.click();
    });
  }

  test('periodic snapshot dots appear on timeline (gray)', async ({ page }) => {
    const snapshotDots = page.locator('app-timeline button.bg-gray-500');
    const count = await snapshotDots.count();

    if (count === 0) {
      test.skip(true, 'No periodic snapshots for today');
      return;
    }

    expect(count).toBeGreaterThan(0);
  });

  test('clicking periodic snapshot dot opens snapshot viewer', async ({ page }) => {
    const snapshotDots = page.locator('app-timeline button.bg-gray-500');
    if (await snapshotDots.count() === 0) {
      test.skip(true, 'No periodic snapshots for today');
      return;
    }

    await clickSnapshotDot(page);

    // Snapshot image should be visible (not video player)
    await expect(page.locator('img[alt="Periodic snapshot"]')).toBeVisible({ timeout: 5000 });
    await expect(page.locator('video')).not.toBeVisible();
  });

  test('snapshot viewer shows purple SNAPSHOT badge', async ({ page }) => {
    const snapshotDots = page.locator('app-timeline button.bg-gray-500');
    if (await snapshotDots.count() === 0) {
      test.skip(true, 'No periodic snapshots for today');
      return;
    }

    await clickSnapshotDot(page);

    await expect(page.locator(':text("SNAPSHOT")')).toBeVisible({ timeout: 5000 });
    const badge = page.locator('[class*="bg-purple"]:has-text("SNAPSHOT")');
    await expect(badge).toBeVisible();
  });

  test('back to live button works from snapshot view', async ({ page }) => {
    const snapshotDots = page.locator('app-timeline button.bg-gray-500');
    if (await snapshotDots.count() === 0) {
      test.skip(true, 'No periodic snapshots for today');
      return;
    }

    await clickSnapshotDot(page);
    await expect(page.locator('img[alt="Periodic snapshot"]')).toBeVisible({ timeout: 5000 });

    await page.locator('button:text("Back to Live")').last().click();

    await expect(page.locator('img[alt="Periodic snapshot"]')).not.toBeVisible();
    await expect(page.locator('.bg-red-600')).toBeVisible({ timeout: 5000 });
  });

  test('snapshot image loads successfully', async ({ page }) => {
    const snapshotDots = page.locator('app-timeline button.bg-gray-500');
    if (await snapshotDots.count() === 0) {
      test.skip(true, 'No periodic snapshots for today');
      return;
    }

    const snapshotResponsePromise = page.waitForResponse(
      (resp: any) => resp.url().includes('/snapshots/'),
      { timeout: 10000 }
    );

    await clickSnapshotDot(page);

    const response = await snapshotResponsePromise;
    console.log('Snapshot URL:', response.url());
    console.log('Snapshot status:', response.status());
    console.log('Content-Type:', response.headers()['content-type']);

    expect(response.status()).toBe(200);
    expect(response.headers()['content-type']).toContain('image');
  });
});
