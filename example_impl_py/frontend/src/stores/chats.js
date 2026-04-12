import { defineStore } from 'pinia'
import { computed, reactive, ref } from 'vue'

export const useChatsStore = defineStore('chats', () => {
  const chats = ref([
    { id: 1, name: 'General', avatar: 'G' },
    { id: 2, name: 'Random', avatar: 'R' },
  ])

  const selectedChatId = ref(null)

  const selectedChat = computed(() => {
    if (!selectedChatId.value) return null
    return chats.value.find(chat => chat.id === selectedChatId.value) || null
  })

  function selectChat(id) {
    selectedChatId.value = id
  }

  function setChats(newChats) {
    chats.value = newChats
  }

  return {
    chats,
    selectedChatId,
    selectedChat,
    selectChat,
    setChats,
  }
})
