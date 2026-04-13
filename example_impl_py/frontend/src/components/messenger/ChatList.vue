<template>
  <div class="chat-list w-[300px] bg-white border-r border-gray-200 flex flex-col h-full">
    <div class="p-4 border-b border-gray-200">
      <h2 class="text-lg font-semibold text-gray-900 mb-2">Chats</h2>
      <input
        v-model="searchQuery"
        type="text"
        placeholder="Search chats..."
        class="w-full px-3 py-2 border border-gray-300 rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-blue-500"
      />
    </div>
    
    <!-- Add Direct Chat button -->
    <button
      v-if="searchQuery && !exactMatchFound"
      class="m-2 p-2 bg-blue-50 text-blue-600 rounded-lg text-sm hover:bg-blue-100 flex items-center gap-2"
      @click="startDirectChat"
    >
      <span>➕</span> Start chat with "{{ searchQuery }}"
    </button>
    
    <div class="flex-1 overflow-y-auto">
      <ChatListItem
        v-for="chat in filteredChats"
        :key="chat.id"
        :chat="chat"
        :is-selected="chat.id === chatStore.selectedChatId"
        :status="chatStore.getStatusForLogin ? chatStore.getStatusForLogin(chat.name) : 'offline'"
        :unread-count="getUnreadCount(chat.id)"
        @select="handleSelectChat(chat.id)"
      />
      <div v-if="filteredChats.length === 0" class="p-4 text-center text-gray-500 text-sm">
        No chats found
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useChatsStore } from '@/stores/chats'
import ChatListItem from './ChatListItem.vue'

const chatStore = useChatsStore()
const searchQuery = ref('')

const emit = defineEmits(['chat-selected', 'start-direct-chat'])

const filteredChats = computed(() => {
  if (!searchQuery.value.trim()) {
    return chatStore.chats
  }
  const query = searchQuery.value.toLowerCase()
  return chatStore.chats.filter(chat => 
    chat.name.toLowerCase().includes(query)
  )
})

const exactMatchFound = computed(() => {
  if (!searchQuery.value.trim()) return false
  const query = searchQuery.value.toLowerCase()
  return chatStore.chats.some(chat => chat.name.toLowerCase() === query)
})

function getUnreadCount(chatId) {
  return 0
}

function handleSelectChat(chatId) {
  chatStore.selectChat(chatId)
  emit('chat-selected', chatId)
}

function startDirectChat() {
  emit('start-direct-chat', searchQuery.value.trim())
}
</script>
