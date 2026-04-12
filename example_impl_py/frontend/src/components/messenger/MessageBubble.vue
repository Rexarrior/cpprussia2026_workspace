<template>
  <div
    class="message-bubble p-3 rounded-lg max-w-[70%] transition-opacity"
    :class="isOwnMessage ? 'bg-blue-500 text-white ml-auto' : 'bg-gray-100 text-gray-900 mr-auto'"
    @mouseenter="showReactions = true"
    @mouseleave="showReactions = false"
  >
    <div v-if="!isOwnMessage" class="author font-semibold text-sm mb-1">
      {{ message.author }}
    </div>
    <div class="text break-words">{{ message.text }}</div>
    <div class="timestamp text-xs mt-1 opacity-70">
      {{ formattedTime }}
    </div>
    <Transition name="fade">
      <div v-if="showReactions" class="reaction-bar-wrapper mt-2">
        <ReactionBar
          :channel-id="message.channel_id"
          :message-id="message.id"
          :reactions="message.reactions || {}"
        />
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useAuthStore } from '@/stores/auth'
import ReactionBar from './ReactionBar.vue'

const props = defineProps({
  message: {
    type: Object,
    required: true,
    validator: (value) => 'id' in value && 'author' in value && 'text' in value
  }
})

const authStore = useAuthStore()
const showReactions = ref(false)

const isOwnMessage = computed(() => {
  if (!authStore.currentUser) return false
  return props.message.author === authStore.currentUser.login
})

const formattedTime = computed(() => {
  if (!props.message.timestamp) return ''
  const date = new Date(props.message.timestamp)
  return date.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit', hour12: false })
})
</script>

<style scoped>
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.2s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
