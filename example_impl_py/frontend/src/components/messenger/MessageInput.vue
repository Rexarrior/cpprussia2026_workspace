<template>
  <div class="message-input border-t border-gray-200 p-3 bg-white">
    <div class="flex items-end gap-3">
      <textarea
        ref="textareaRef"
        v-model="messageText"
        class="flex-1 resize-none border border-gray-300 rounded-lg px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent"
        placeholder="Type a message..."
        rows="1"
        @keydown.enter.exact.prevent="handleSend"
        @input="autoResize"
      ></textarea>
      <button
        class="send-btn px-4 py-2 bg-blue-500 text-white rounded-lg hover:bg-blue-600 transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
        :disabled="!messageText.trim()"
        @click="handleSend"
      >
        Send
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref, nextTick } from 'vue'
import { useMessagesStore } from '@/stores/messages'
import { useAuthStore } from '@/stores/auth'

const messagesStore = useMessagesStore()
const authStore = useAuthStore()

const messageText = ref('')
const textareaRef = ref(null)

const props = defineProps({
  channelId: {
    type: Number,
    required: true
  }
})

const emit = defineEmits(['message-sent'])

async function handleSend() {
  const text = messageText.value.trim()
  if (!text) return

  await messagesStore.sendMessage(props.channelId, text, authStore.currentUser.login)

  messageText.value = ''
  await nextTick()
  autoResize()
  emit('message-sent')
}

function autoResize() {
  if (textareaRef.value) {
    textareaRef.value.style.height = 'auto'
    textareaRef.value.style.height = textareaRef.value.scrollHeight + 'px'
  }
}
</script>
