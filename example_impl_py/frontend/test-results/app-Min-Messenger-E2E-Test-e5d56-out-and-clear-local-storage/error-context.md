# Instructions

- Following Playwright test failed.
- Explain why, be concise, respect Playwright best practices.
- Provide a snippet of code with the fix, if possible.

# Test info

- Name: app.spec.js >> Min Messenger E2E Tests >> should logout and clear local storage
- Location: tests/e2e/app.spec.js:174:3

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
      - textbox [ref=e11]: logouttest
    - generic [ref=e12]:
      - generic [ref=e13]: Name
      - textbox [ref=e14]: Logout Test
    - generic [ref=e15]:
      - generic [ref=e16]: Email
      - textbox [ref=e17]: logout@example.com
    - generic [ref=e18]:
      - generic [ref=e19]: Phone
      - textbox [ref=e20]: "+7777777777"
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
  101 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
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
> 184 |     await page.waitForURL(/.*messages/, { timeout: 5000 });
      |                ^ TimeoutError: page.waitForURL: Timeout 5000ms exceeded.
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