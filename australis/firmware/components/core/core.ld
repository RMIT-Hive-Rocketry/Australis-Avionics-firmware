SECTIONS
{
  .shell_vector : 
  {
	  __shell_vector_start = .;
	  KEEP(*(.shell_*));
	  __shell_vector_end = .;
  } >FLASH

  .topic_vector : 
  {
	  __topic_vector_start = .;
	  KEEP(*(.topic_*));
	  __topic_vector_end = .;
  } >RAM
}
