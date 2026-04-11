import { createRouter, createWebHistory } from 'vue-router'
import Login from '../views/Login.vue'
import Register from '../views/Register.vue'
import Messages from '../views/Messages.vue'
import Notifications from '../views/Notifications.vue'
import Files from '../views/Files.vue'
import Reactions from '../views/Reactions.vue'
import Status from '../views/Status.vue'

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/login', component: Login },
    { path: '/register', component: Register },
    { path: '/messages', component: Messages },
    { path: '/notifications', component: Notifications },
    { path: '/files', component: Files },
    { path: '/reactions', component: Reactions },
    { path: '/status', component: Status }
  ]
})

export default router
