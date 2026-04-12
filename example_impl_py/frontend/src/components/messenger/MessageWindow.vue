<template>
  <div class="message-window flex flex-col h-full bg-gray-50">
    <div class="header p-4 bg-white border-b border-gray-200">
      <h2 class="text-lg font-semibold text-gray-900">
        {{ chatName }}
      </h2>
    </div>
    <div
      ref="messagesContainer"
      class="messages-container flex-1 overflow-y-auto p-4 space-y-4"
    >
      <MessageBubble
        v-for="msg in currentMessages"
        :key="msg.id"
        :message="msg"
      />
    </div>
    <MessageInput :channel-id="props.channelId" @message-sent="scrollToBottom" />
  </div>
</template>

<script setup>
import { ref, computed, watch, nextTick } from 'vue'
import { useChatsStore } from '@/stores/chats'
import { useMessagesStore } from '@/stores/messages'
import MessageBubble from './MessageBubble.vue'
import MessageInput from './MessageInput.vue'

const props = defineProps({
  channelId: {
    type: [Number, String],
    required: true
  }
})

const chatsStore = useChatsStore()
const messagesStore = useMessagesStore()
const messagesContainer = ref(null)

const chatName = computed(() => {
  const chat = chatsStore.chats.find(c => c.id === props.channelId)
  return chat?.name || 'Chat'
})

const currentMessages = computed(() => {
  return messagesStore.messages[props.channelId] || []
})

watch(
  () => currentMessages.value.length,
  () => {
    nextTick(() => scrollToBottom())
  }
)

watch(
  () => props.channelId,
  async (newId) => {
    await messagesStore.fetchMessages(newId)
    nextTick(() => scrollToBottom())
  },
  { immediate: true }
)

function scrollToBottom() {
  if (messagesContainer.value) {
    messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight
  }
}
</script>
