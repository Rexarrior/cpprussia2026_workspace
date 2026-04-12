import { test, expect } from '@playwright/test';

const BASE_URL = 'http://localhost';

async function registerAndLogin(page, username) {
  await page.goto(`${BASE_URL}/register`);
  
  // Wait for form to be visible
  await page.waitForSelector('input[type="text"]', { timeout: 5000 });
  
  const inputs = page.locator('input');
  await inputs.nth(0).fill(username);
  await inputs.nth(1).fill('Test User');
  await inputs.nth(2).fill(`${username}@example.com`);
  await inputs.nth(3).fill('+1234567890');
  await inputs.nth(4).fill('password123');
  
  // Capture console messages
  page.on('console', msg => {
    console.log('Browser console:', msg.text());
  });
  
  // Set up network monitoring
  let apiResponse = null;
  page.on('response', response => {
    if (response.url().includes('/api/auth/')) {
      console.log(`API Response: ${response.url()} - Status: ${response.status()}`);
      apiResponse = response;
    }
  });
  
  // Click submit
  await page.click('button[type="submit"]');
  
  // Wait for either success message or error
  try {
    await page.waitForSelector('.success, .error', { timeout: 5000 });
  } catch (e) {
    console.log('No success or error message appeared');
  }
  
  // Check current URL
  const currentUrl = page.url();
  console.log('Current URL after submit:', currentUrl);
  
  // If we got a success message, wait for navigation
  const successEl = page.locator('.success');
  if (await successEl.isVisible()) {
    console.log('Success message visible, waiting for navigation...');
    await page.waitForURL(/.*messages|.*messenger/, { timeout: 10000 });
    return;
  }
  
  // If we got an error message, log it
  const errorEl = page.locator('.error');
  if (await errorEl.isVisible()) {
    const errorText = await errorEl.textContent();
    console.log('Registration error:', errorText);
  }
  
  // If still on register, wait for URL change with longer timeout
  if (currentUrl.includes('/register')) {
    try {
      await page.waitForURL(/.*messages|.*messenger/, { timeout: 10000 });
    } catch (e) {
      // Take screenshot for debugging
      await page.screenshot({ path: `error-${username}.png` });
      const errorEl = page.locator('.error');
      const errorText = await errorEl.isVisible() ? await errorEl.textContent() : 'none';
      throw new Error(`Registration failed, still on: ${currentUrl}, error: ${errorText}`);
    }
  }
}

test.describe('Messenger E2E Tests', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto(`${BASE_URL}/`);
    await page.evaluate(() => localStorage.clear());
    await page.reload();
    // Wait for page to fully load
    await page.waitForLoadState('networkidle');
  });

  test('messenger redirect when not authenticated', async ({ page }) => {
    await page.goto(`${BASE_URL}/messenger`);
    await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
  });

  test('messenger accessible when logged in', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await expect(page).toHaveURL(/.*messenger/, { timeout: 5000 });
  });

  test('chat list displays chats', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await expect(page.locator('text=General')).toBeVisible();
    await expect(page.locator('text=Random')).toBeVisible();
    await expect(page.locator('.chat-list')).toBeVisible();
    // Verify chat list has the correct styling class
    await expect(page.locator('.chat-list')).toHaveClass(/w-\[300px\]/);
  });

  test('can select a chat', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    await expect(page.locator('.chat-list-item.bg-blue-50')).toBeVisible();
    await expect(page.locator('.message-window .header h2')).toHaveText('General');
  });

  test('message input field works', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    const textarea = page.locator('textarea[placeholder="Type a message..."]');
    await textarea.fill('Hello World');
    await expect(textarea).toHaveValue('Hello World');
  });

  test('can send message with Enter', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    const textarea = page.locator('textarea[placeholder="Type a message..."]');
    const testMessage = `Test message ${Date.now()}`;
    await textarea.fill(testMessage);
    await page.keyboard.press('Enter');
    await page.waitForResponse(response => response.url().includes('/api/messages') && response.status() === 200);
    await expect(textarea).toHaveValue('');
  });

  test('message appears in list after sending', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    const textarea = page.locator('textarea[placeholder="Type a message..."]');
    const testMessage = `Test message ${Date.now()}`;
    await textarea.fill(testMessage);
    await page.keyboard.press('Enter');
    await page.waitForResponse(response => response.url().includes('/api/messages') && response.status() === 200);
    await expect(page.locator('.message-bubble').filter({ hasText: testMessage })).toBeVisible({ timeout: 5000 });
  });

  test('reaction bar appears on hover', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    const textarea = page.locator('textarea[placeholder="Type a message..."]');
    const testMessage = `Message for reaction ${Date.now()}`;
    await textarea.fill(testMessage);
    await page.keyboard.press('Enter');
    await page.waitForResponse(response => response.url().includes('/api/messages') && response.status() === 200);
    const bubble = page.locator('.message-bubble').filter({ hasText: testMessage });
    await bubble.hover();
    await expect(page.locator('.reaction-bar')).toBeVisible({ timeout: 3000 });
  });

  test('clicking reaction updates counter', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await page.locator('text=General').first().click();
    const textarea = page.locator('textarea[placeholder="Type a message..."]');
    const testMessage = `Message for reaction test ${Date.now()}`;
    await textarea.fill(testMessage);
    await page.keyboard.press('Enter');
    await page.waitForResponse(response => response.url().includes('/api/messages') && response.status() === 200);
    const bubble = page.locator('.message-bubble').filter({ hasText: testMessage });
    await bubble.hover();
    const reactionBar = page.locator('.reaction-bar');
    await expect(reactionBar).toBeVisible({ timeout: 3000 });
    const thumbsUp = reactionBar.locator('button').filter({ hasText: '👍' });
    await thumbsUp.click();
    await page.waitForResponse(response => response.url().includes('/api/reactions') && response.status() === 200);
    const counter = thumbsUp.locator('span').last();
    await expect(parseInt(await counter.textContent())).toBeGreaterThanOrEqual(1);
  });

  test('messenger layout structure', async ({ page }) => {
    const username = `user_${Date.now()}`;
    await registerAndLogin(page, username);
    await page.goto(`${BASE_URL}/messenger`);
    await expect(page.locator('.messenger-layout')).toBeVisible();
    // Verify chat list has correct width class
    await expect(page.locator('.chat-list')).toHaveClass(/w-\[300px\]/);
    await expect(page.locator('.message-area')).toBeVisible();
  });
});
