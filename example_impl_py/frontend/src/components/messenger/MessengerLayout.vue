<template>
  <div class="messenger-layout flex h-screen w-full overflow-hidden">
    <ChatList @chat-selected="handleChatSelected" />
    <div class="message-area flex-1">
      <MessageWindow
        v-if="chatStore.selectedChatId"
        :key="chatStore.selectedChatId"
        :channel-id="chatStore.selectedChatId"
      />
      <div v-else class="flex items-center justify-center h-full text-gray-500">
        Select a chat to start messaging
      </div>
    </div>
  </div>
</template>

<script setup>
import { useChatsStore } from '@/stores/chats'
import ChatList from './ChatList.vue'
import MessageWindow from './MessageWindow.vue'

const chatStore = useChatsStore()

function handleChatSelected(chatId) {
  console.log('Chat selected:', chatId)
}
</script>
