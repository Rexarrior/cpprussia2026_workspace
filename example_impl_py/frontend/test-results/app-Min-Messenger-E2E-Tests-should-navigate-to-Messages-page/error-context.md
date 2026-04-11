# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: app.spec.js >> Min Messenger E2E Tests >> should navigate to Messages page
- Location: tests/e2e/app.spec.js:92:3

# Error details

```
TimeoutError: page.waitForURL: Timeout 5000ms exceeded.
=========================== logs ===========================
waiting for navigation until "load"
============================================================
```

# Page snapshot

```yaml
- generic [ref=e5]:
  - heading "Register" [level=2] [ref=e6]
  - generic [ref=e7]: Registration failed
  - generic [ref=e8]:
    - generic [ref=e9]:
      - generic [ref=e10]: Login
      - textbox [ref=e11]: msgtnav
    - generic [ref=e12]:
      - generic [ref=e13]: Name
      - textbox [ref=e14]: Msg Nav
    - generic [ref=e15]:
      - generic [ref=e16]: Email
      - textbox [ref=e17]: navmsg@example.com
    - generic [ref=e18]:
      - generic [ref=e19]: Phone
      - textbox [ref=e20]: "+2222222222"
    - generic [ref=e21]:
      - generic [ref=e22]: Password
      - textbox [ref=e23]: password123
    - button "Register" [ref=e24] [cursor=pointer]
  - paragraph [ref=e25]:
    - text: Already have an account?
    - link "Login" [ref=e26] [cursor=pointer]:
      - /url: /login
```

# Test source

```ts
  1   | import { test, expect } from '@playwright/test';
  2   | 
  3   | test.describe('Min Messenger E2E Tests', () => {
  4   |   test.beforeEach(async ({ page }) => {
  5   |     // Clear local storage before each test
  6   |     await page.goto('http://localhost:3000');
  7   |     await page.evaluate(() => localStorage.clear());
  8   |     await page.reload();
  9   |   });
  10  | 
  11  |   test('should load login page', async ({ page }) => {
  12  |     await page.goto('http://localhost:3000/login');
  13  |     await expect(page.locator('h2')).toHaveText('Login');
  14  |     await expect(page.locator('input[type="text"]')).toBeVisible();
  15  |     await expect(page.locator('input[type="password"]')).toBeVisible();
  16  |   });
  17  | 
  18  |   test('should navigate to register page', async ({ page }) => {
  19  |     await page.goto('http://localhost:3000/login');
  20  |     await page.click('text=Register');
  21  |     await expect(page.locator('h2')).toHaveText('Register');
  22  |   });
  23  | 
  24  |   test('should show validation errors on empty form', async ({ page }) => {
  25  |     await page.goto('http://localhost:3000/login');
  26  |     await page.click('button[type="submit"]');
  27  |     // Browser validation should prevent submission
  28  |   });
  29  | 
  30  |   test('should register a new user', async ({ page }) => {
  31  |     await page.goto('http://localhost:3000/register');
  32  |     
  33  |     await page.fill('input[type="text"]', 'testuser');
  34  |     await page.fill('input[placeholder="Type a message..."]').first().locator('..').locator('input').first();
  35  |     
  36  |     // Fill all fields
  37  |     const inputs = page.locator('input');
  38  |     await inputs.nth(0).fill('testuser'); // login
  39  |     await inputs.nth(1).fill('Test User'); // name
  40  |     await inputs.nth(2).fill('test@example.com'); // email
  41  |     await inputs.nth(3).fill('+1234567890'); // phone
  42  |     await inputs.nth(4).fill('password123'); // password
  43  |     
  44  |     await page.click('button[type="submit"]');
  45  |     
  46  |     // Should redirect to messages after success
  47  |     await expect(page).toHaveURL(/.*messages/, { timeout: 5000 });
  48  |   });
  49  | 
  50  |   test('should login with existing user', async ({ page }) => {
  51  |     // First register
  52  |     await page.goto('http://localhost:3000/register');
  53  |     const inputs = page.locator('input');
  54  |     await inputs.nth(0).fill('logintest');
  55  |     await inputs.nth(1).fill('Login Test');
  56  |     await inputs.nth(2).fill('login@example.com');
  57  |     await inputs.nth(3).fill('+0987654321');
  58  |     await inputs.nth(4).fill('password123');
  59  |     await page.click('button[type="submit"]');
  60  |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  61  |     
  62  |     // Logout
  63  |     await page.click('text=Logout');
  64  |     await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
  65  |     
  66  |     // Login again
  67  |     await page.fill('input[type="text"]', 'logintest');
  68  |     await page.fill('input[type="password"]', 'password123');
  69  |     await page.click('button[type="submit"]');
  70  |     await expect(page).toHaveURL(/.*messages/, { timeout: 5000 });
  71  |   });
  72  | 
  73  |   test('should show navbar when logged in', async ({ page }) => {
  74  |     await page.goto('http://localhost:3000/register');
  75  |     const inputs = page.locator('input');
  76  |     await inputs.nth(0).fill('navtest');
  77  |     await inputs.nth(1).fill('Nav Test');
  78  |     await inputs.nth(2).fill('nav@example.com');
  79  |     await inputs.nth(3).fill('+1111111111');
  80  |     await inputs.nth(4).fill('password123');
  81  |     await page.click('button[type="submit"]');
  82  |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  83  |     
  84  |     // Check navbar links
  85  |     await expect(page.locator('text=Messages')).toBeVisible();
  86  |     await expect(page.locator('text=Notifications')).toBeVisible();
  87  |     await expect(page.locator('text=Files')).toBeVisible();
  88  |     await expect(page.locator('text=Reactions')).toBeVisible();
  89  |     await expect(page.locator('text=Status')).toBeVisible();
  90  |   });
  91  | 
  92  |   test('should navigate to Messages page', async ({ page }) => {
  93  |     await page.goto('http://localhost:3000/register');
  94  |     const inputs = page.locator('input');
  95  |     await inputs.nth(0).fill('msgtnav');
  96  |     await inputs.nth(1).fill('Msg Nav');
  97  |     await inputs.nth(2).fill('navmsg@example.com');
  98  |     await inputs.nth(3).fill('+2222222222');
  99  |     await inputs.nth(4).fill('password123');
  100 |     await page.click('button[type="submit"]');
> 101 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
      |                ^ TimeoutError: page.waitForURL: Timeout 5000ms exceeded.
  102 |     
  103 |     await page.click('text=Messages');
  104 |     await expect(page.locator('h2')).toHaveText('Messages');
  105 |     await expect(page.locator('select')).toBeVisible();
  106 |   });
  107 | 
  108 |   test('should navigate to Notifications page', async ({ page }) => {
  109 |     await page.goto('http://localhost:3000/register');
  110 |     const inputs = page.locator('input');
  111 |     await inputs.nth(0).fill('notiftest');
  112 |     await inputs.nth(1).fill('Notif Test');
  113 |     await inputs.nth(2).fill('notif@example.com');
  114 |     await inputs.nth(3).fill('+3333333333');
  115 |     await inputs.nth(4).fill('password123');
  116 |     await page.click('button[type="submit"]');
  117 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  118 |     
  119 |     await page.click('text=Notifications');
  120 |     await expect(page.locator('h2')).toHaveText('Notifications');
  121 |   });
  122 | 
  123 |   test('should navigate to Files page', async ({ page }) => {
  124 |     await page.goto('http://localhost:3000/register');
  125 |     const inputs = page.locator('input');
  126 |     await inputs.nth(0).fill('filetest');
  127 |     await inputs.nth(1).fill('File Test');
  128 |     await inputs.nth(2).fill('file@example.com');
  129 |     await inputs.nth(3).fill('+4444444444');
  130 |     await inputs.nth(4).fill('password123');
  131 |     await page.click('button[type="submit"]');
  132 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  133 |     
  134 |     await page.click('text=Files');
  135 |     await expect(page.locator('h2')).toHaveText('Files');
  136 |   });
  137 | 
  138 |   test('should navigate to Reactions page', async ({ page }) => {
  139 |     await page.goto('http://localhost:3000/register');
  140 |     const inputs = page.locator('input');
  141 |     await inputs.nth(0).fill('reacttest');
  142 |     await inputs.nth(1).fill('React Test');
  143 |     await inputs.nth(2).fill('react@example.com');
  144 |     await inputs.nth(3).fill('+5555555555');
  145 |     await inputs.nth(4).fill('password123');
  146 |     await page.click('button[type="submit"]');
  147 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  148 |     
  149 |     await page.click('text=Reactions');
  150 |     await expect(page.locator('h2')).toHaveText('Reactions');
  151 |   });
  152 | 
  153 |   test('should navigate to Status page', async ({ page }) => {
  154 |     await page.goto('http://localhost:3000/register');
  155 |     const inputs = page.locator('input');
  156 |     await inputs.nth(0).fill('statustest');
  157 |     await inputs.nth(1).fill('Status Test');
  158 |     await inputs.nth(2).fill('status@example.com');
  159 |     await inputs.nth(3).fill('+6666666666');
  160 |     await inputs.nth(4).fill('password123');
  161 |     await page.click('button[type="submit"]');
  162 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  163 |     
  164 |     await page.click('text=Status');
  165 |     await expect(page.locator('h2')).toHaveText('Status');
  166 |   });
  167 | 
  168 |   test('should redirect to login when accessing protected route', async ({ page }) => {
  169 |     await page.goto('http://localhost:3000/messages');
  170 |     // Should redirect to login
  171 |     await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
  172 |   });
  173 | 
  174 |   test('should logout and clear local storage', async ({ page }) => {
  175 |     // Login first
  176 |     await page.goto('http://localhost:3000/register');
  177 |     const inputs = page.locator('input');
  178 |     await inputs.nth(0).fill('logouttest');
  179 |     await inputs.nth(1).fill('Logout Test');
  180 |     await inputs.nth(2).fill('logout@example.com');
  181 |     await inputs.nth(3).fill('+7777777777');
  182 |     await inputs.nth(4).fill('password123');
  183 |     await page.click('button[type="submit"]');
  184 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
  185 |     
  186 |     // Verify local storage has token
  187 |     const token = await page.evaluate(() => localStorage.getItem('token'));
  188 |     expect(token).toBeTruthy();
  189 |     
  190 |     // Logout
  191 |     await page.click('text=Logout');
  192 |     
  193 |     // Token should be cleared
  194 |     const tokenAfterLogout = await page.evaluate(() => localStorage.getItem('token'));
  195 |     expect(tokenAfterLogout).toBeNull();
  196 |   });
  197 | });
  198 | 
```