SECTIONS
{
  .topic_vector :
  {
    __topic_vector_start = .;
    *(.topic_*);
    __topic_vector_end = .;
  } >RAM
}
