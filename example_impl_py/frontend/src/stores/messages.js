import { defineStore } from 'pinia'
import { reactive, ref, computed } from 'vue'
import { messagingApi } from '@/api'

export const useMessagesStore = defineStore('messages', () => {
  const messages = reactive({})
  const pollingInterval = ref(null)

  function messagesForChannel(channelId) {
    return computed(() => messages[channelId] || [])
  }

  async function fetchMessages(channelId) {
    try {
      const token = localStorage.getItem('token')
      const login = localStorage.getItem('login')
      const name = localStorage.getItem('name')
      const currentUser = { token, login, name }
      
      const response = await messagingApi.getMessages(channelId, '0', String(Date.now()), currentUser)
      messages[channelId] = response.data.messages || []
      startPolling(channelId)
    } catch (error) {
      console.error('Failed to fetch messages:', error)
    }
  }

  async function sendMessage(channelId, text, author) {
    try {
      const token = localStorage.getItem('token')
      const currentUser = {
        token: token,
        login: author,
        name: author,
      }

      const response = await messagingApi.sendMessage({
        current_user: currentUser,
        channel_id: channelId,
        message: text,
      })
      addMessage(response.data)
      return response.data
    } catch (error) {
      console.error('Failed to send message:', error)
      throw error
    }
  }

  function addMessage(message) {
    const channelId = message.channel_id
    if (!messages[channelId]) {
      messages[channelId] = []
    }
    messages[channelId].push(message)
  }

  function startPolling(channelId) {
    stopPolling()
    pollingInterval.value = setInterval(() => {
      fetchMessages(channelId)
    }, 5000)
  }

  function stopPolling() {
    if (pollingInterval.value) {
      clearInterval(pollingInterval.value)
      pollingInterval.value = null
    }
  }

  return {
    messages,
    pollingInterval,
    messagesForChannel,
    fetchMessages,
    sendMessage,
    addMessage,
    startPolling,
    stopPolling,
  }
})
