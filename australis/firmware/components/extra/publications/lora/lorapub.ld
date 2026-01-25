SECTIONS
{
  .topic_lora :
  {
    __lora_subscriptions_start = .;
    *(.lora_subscription);
    __lora_subscriptions_end = .;
  } >RAM
}
