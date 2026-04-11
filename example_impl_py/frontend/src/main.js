import { createApp } from 'vue'
import { createRouter, createWebHistory } from 'vue-router'
import App from './App.vue'

import Login from './views/Login.vue'
import Register from './views/Register.vue'
import Messages from './views/Messages.vue'
import Notifications from './views/Notifications.vue'
import Files from './views/Files.vue'
import Reactions from './views/Reactions.vue'
import Status from './views/Status.vue'

const routes = [
  { path: '/', redirect: '/login' },
  { path: '/login', component: Login },
  { path: '/register', component: Register },
  { path: '/messages', component: Messages },
  { path: '/notifications', component: Notifications },
  { path: '/files', component: Files },
  { path: '/reactions', component: Reactions },
  { path: '/status', component: Status },
]

const router = createRouter({
  history: createWebHistory(),
  routes,
})

export const currentUser = {
  token: localStorage.getItem('token') || null,
  login: localStorage.getItem('login') || null,
  name: localStorage.getItem('name') || null,
}

export function setCurrentUser(user) {
  currentUser.token = user.token
  currentUser.login = user.login
  currentUser.name = user.name
  localStorage.setItem('token', user.token)
  localStorage.setItem('login', user.login)
  localStorage.setItem('name', user.name)
}

export function clearCurrentUser() {
  currentUser.token = null
  currentUser.login = null
  currentUser.name = null
  localStorage.removeItem('token')
  localStorage.removeItem('login')
  localStorage.removeItem('name')
}

const app = createApp(App)
app.use(router)
app.mount('#app')
