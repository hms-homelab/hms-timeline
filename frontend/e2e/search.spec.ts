import { test, expect } from '@playwright/test';

test.describe('Search Feature', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/');
    await expect(page.locator('app-camera-tabs')).toBeVisible({ timeout: 10000 });
  });

  test('search input is visible', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await expect(searchInput).toBeVisible();
  });

  test('typing in search box shows results', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await searchInput.fill('person');
    await searchInput.press('Enter');

    // Wait for search results or "No results" message
    await expect(
      page.locator(':text("results across all cameras"), :text("No results found")')
    ).toBeVisible({ timeout: 10000 });
  });

  test('class filter chips are displayed', async ({ page }) => {
    // Verify all class filter chips are present
    for (const cls of ['person', 'dog', 'cat', 'car', 'package']) {
      await expect(page.locator(`button:text-is("${cls}")`)).toBeVisible();
    }
  });

  test('clicking class chip triggers search with class filter', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await searchInput.fill('motion');

    // Click the "person" class chip
    await page.locator('button:text-is("person")').click();

    // Search should be triggered — wait for results
    await expect(
      page.locator(':text("results across all cameras"), :text("No results found")')
    ).toBeVisible({ timeout: 10000 });
  });

  test('search results show event and snapshot types', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await searchInput.fill('person');
    await searchInput.press('Enter');

    // Wait for results
    await page.waitForTimeout(2000);

    // Check for result cards (either event or snapshot type indicators)
    const resultCards = page.locator('[class*="cursor-pointer"][class*="border"]').filter({
      has: page.locator(':text("AM"), :text("PM")')
    });

    const count = await resultCards.count();
    if (count > 0) {
      // At least one result should be visible
      await expect(resultCards.first()).toBeVisible();
    }
  });

  test('clearing search returns to normal view', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await searchInput.fill('person');
    await searchInput.press('Enter');

    // Wait for search mode
    await page.waitForTimeout(1000);

    // Click the clear (X) button
    const clearButton = page.locator('input[placeholder*="Search events"] + button, input[placeholder*="Search events"] ~ button').first();
    if (await clearButton.isVisible()) {
      await clearButton.click();
    } else {
      // Alternatively, clear the input manually
      await searchInput.clear();
    }

    // Date picker / timeline should reappear (they're hidden in search mode)
    await expect(page.locator('app-date-picker')).toBeVisible({ timeout: 5000 });
    await expect(page.locator('app-timeline')).toBeVisible({ timeout: 5000 });
  });

  test('clicking search result plays recording or shows snapshot', async ({ page }) => {
    const searchInput = page.locator('input[placeholder*="Search events"]');
    await searchInput.fill('person');
    await searchInput.press('Enter');

    // Wait for results
    await page.waitForTimeout(2000);

    const firstResult = page.locator('[class*="cursor-pointer"][class*="rounded-xl"]').first();
    const hasResults = await firstResult.count() > 0;
    if (!hasResults) {
      test.skip(true, 'No search results available');
      return;
    }

    await firstResult.click();

    // Either video or snapshot viewer should appear
    await expect(
      page.locator('video, img[alt="Periodic snapshot"]').first()
    ).toBeVisible({ timeout: 5000 });
  });
});
