<template>
  <div class="chat-list w-[300px] bg-white border-r border-gray-200 flex flex-col h-full">
    <div class="p-4 border-b border-gray-200">
      <h2 class="text-lg font-semibold text-gray-900">Chats</h2>
    </div>
    <div class="flex-1 overflow-y-auto">
      <ChatListItem
        v-for="chat in chatStore.chats"
        :key="chat.id"
        :chat="chat"
        :is-selected="chat.id === chatStore.selectedChatId"
        :status="chatStore.getStatusForLogin ? chatStore.getStatusForLogin(chat.name) : 'offline'"
        :unread-count="getUnreadCount(chat.id)"
        @select="handleSelectChat(chat.id)"
      />
    </div>
  </div>
</template>

<script setup>
import { useChatsStore } from '@/stores/chats'
import ChatListItem from './ChatListItem.vue'

const chatStore = useChatsStore()

const emit = defineEmits(['chat-selected'])

function getUnreadCount(chatId) {
  return 0
}

function handleSelectChat(chatId) {
  chatStore.selectChat(chatId)
  emit('chat-selected', chatId)
}
</script>
