SECTIONS
{
  .topic_can :
  {
    __can_subscriptions_start = .;
    *(.can_subscription);
    __can_subscriptions_end = .;
  } >RAM
}
