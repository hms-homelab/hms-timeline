import { test, expect } from '@playwright/test';

test.describe('Timeline Basics', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });
  });

  test('camera tabs render with at least one camera', async ({ page }) => {
    const tabs = page.locator('app-camera-tabs button, app-camera-tabs a');
    await expect(tabs.first()).toBeVisible();
    const tabCount = await tabs.count();
    expect(tabCount).toBeGreaterThanOrEqual(1);
  });

  test('switching between cameras updates view', async ({ page }) => {
    const tabs = page.locator('app-camera-tabs button, app-camera-tabs a');
    const tabCount = await tabs.count();

    if (tabCount < 2) {
      test.skip(true, 'Only one camera available');
      return;
    }

    // Click second camera tab
    await tabs.nth(1).click();

    // Wait for events to reload
    await page.waitForTimeout(1500);

    // Timeline and event list should still be visible
    await expect(page.locator('app-timeline')).toBeVisible();
  });

  test('date picker is visible and functional', async ({ page }) => {
    const datePicker = page.locator('app-date-picker');
    await expect(datePicker).toBeVisible();

    // Date picker should have navigation buttons (previous/next day)
    const buttons = datePicker.locator('button');
    const buttonCount = await buttons.count();
    expect(buttonCount).toBeGreaterThanOrEqual(1);
  });

  test('date picker navigation changes date and reloads events', async ({ page }) => {
    // Find a "previous day" button in the date picker
    const datePicker = page.locator('app-date-picker');
    const prevButton = datePicker.locator('button').first();

    await prevButton.click();
    // Events should reload — wait a moment
    await page.waitForTimeout(1500);

    // Timeline should still be visible
    await expect(page.locator('app-timeline')).toBeVisible();
  });

  test('timeline renders 24-hour markers', async ({ page }) => {
    const timeline = page.locator('app-timeline');
    await expect(timeline).toBeVisible();

    // Should have hour labels (00:00 through 23:00)
    await expect(timeline.locator(':text("00:00")')).toBeVisible();
    await expect(timeline.locator(':text("12:00")')).toBeVisible();
  });

  test('timeline shows event dots when events exist', async ({ page }) => {
    // Check if there are events
    const eventList = page.locator('app-event-list');
    const hasEvents = await eventList.count() > 0;

    if (!hasEvents) {
      test.skip(true, 'No events for today');
      return;
    }

    // Timeline should have blue event dots (buttons inside the timeline with bg-blue-500)
    const eventDots = page.locator('app-timeline button.bg-blue-500, app-timeline button[class*="bg-blue"]');
    const dotCount = await eventDots.count();
    expect(dotCount).toBeGreaterThan(0);
  });

  test('clicking timeline event dot selects the event', async ({ page }) => {
    const eventDots = page.locator('app-timeline button[class*="bg-blue"]');
    const hasDots = await eventDots.count() > 0;

    if (!hasDots) {
      test.skip(true, 'No event dots on timeline');
      return;
    }

    await eventDots.first().click();

    // Should switch to recording mode — video element should appear
    await expect(page.locator('video')).toBeVisible({ timeout: 5000 });
  });
});
