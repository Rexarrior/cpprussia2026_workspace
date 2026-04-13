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
      const serverMessages = response.data.messages || []
      
      if (!messages[channelId]) {
        messages[channelId] = []
      }
      
      const mergedMessages = []
      const seenIds = new Set()
      
      for (const msg of messages[channelId]) {
        const id = msg.message_id || msg.timestamp
        if (!seenIds.has(id)) {
          seenIds.add(id)
          mergedMessages.push(msg)
        }
      }
      
      for (const serverMsg of serverMessages) {
        const id = serverMsg.message_id
        if (seenIds.has(id)) {
          continue
        }
        seenIds.add(id)
        mergedMessages.push(serverMsg)
      }
      
      messages[channelId] = mergedMessages
      startPolling(channelId)
    } catch (error) {
      console.error('Failed to fetch messages:', error)
    }
  }

  async function sendMessage(channelId, text, author, fileUri = null, fileName = null) {
    const timestamp = Date.now()
    const tempId = `temp_${timestamp}`
    
    const message = {
      message_id: tempId,
      channel_id: channelId,
      text: text,
      author: author,
      timestamp: timestamp,
      pending: true,
      file_uri: fileUri,
      file_name: fileName
    }
    
    if (!messages[channelId]) {
      messages[channelId] = []
    }
    messages[channelId].push(message)
    
    try {
      const token = localStorage.getItem('token')
      const name = localStorage.getItem('name')
      const currentUser = {
        token: token,
        login: author,
        name: name || author,
      }

      const response = await messagingApi.sendMessage({
        current_user: currentUser,
        channel_id: channelId,
        message: text,
      })
      
      message.message_id = response.data.message_id
      message.pending = false
      return message
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