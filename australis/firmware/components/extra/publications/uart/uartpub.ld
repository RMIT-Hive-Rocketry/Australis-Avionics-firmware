SECTIONS
{
  .topic_uart :
  {
    __uart_subscriptions_start = .;
    *(.uart_subscription);
    __uart_subscriptions_end = .;
  } >RAM
}
