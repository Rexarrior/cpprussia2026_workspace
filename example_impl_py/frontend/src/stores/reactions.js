import { defineStore } from 'pinia'
import { computed, reactive } from 'vue'
import { reactionsApi } from '@/api'

export const useReactionsStore = defineStore('reactions', () => {
  const reactions = reactive({})

  const VALID_REACTIONS = ['like', 'dislike', 'heart', 'fire', 'okay', 'LOL', 'smile']

  function getReactionKey(channelId, messageId) {
    return `${channelId}-${messageId}`
  }

  async function fetchReactions(channelId, messageId) {
    try {
      const response = await reactionsApi.get(channelId, messageId)
      const key = getReactionKey(channelId, messageId)
      reactions[key] = response.data || {}
    } catch (error) {
      console.error('Failed to fetch reactions:', error)
    }
  }

  async function toggleReaction(channelId, messageId, reactionType) {
    if (!VALID_REACTIONS.includes(reactionType)) {
      throw new Error(`Invalid reaction type. Must be one of: ${VALID_REACTIONS.join(', ')}`)
    }
    try {
      const key = getReactionKey(channelId, messageId)
      await reactionsApi.trigger({
        channel_id: channelId,
        message_id: messageId,
        reaction_type: reactionType,
      })
      if (!reactions[key]) {
        reactions[key] = {}
      }
      reactions[key][reactionType] = (reactions[key][reactionType] || 0) + 1
    } catch (error) {
      console.error('Failed to toggle reaction:', error)
      throw error
    }
  }

  return {
    reactions,
    toggleReaction,
    fetchReactions,
    getReactionKey,
  }
})
