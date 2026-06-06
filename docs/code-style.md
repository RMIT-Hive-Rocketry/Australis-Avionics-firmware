# Code Style

While programmers may have their own preferences when it comes to the formatting
of code, it's important for a project to have an overall definition of style.
This is not merely for aesthetics, but because a consistent style is an aid to
the reader's comprehension.

Many style rules can be handled automatically by your text editor. There is a
file at `australis/.clang-format` that defines simple style rules. The rest of
this document deals with more complicated style rules.



# Basic Practices

- Apply a soft limit of 80 characters to a line. Hard limit of 100 characters.
- Keep lines simple. Avoid convoluted expressions, don't try pack everything
  on one line. Break up large expressions with intermediate variables and line
  breaks. Avoid ternary operators, as they are rarely a better expression, and
  may rather lead to another programmer cursing your name six months later.



# Naming

Variable and function names are important for clarity and recall. Ideally, one
should be able to understand is happening in some function, that is using other
functions they may not have seen before.

- When naming functions, use underscore for spacing, and regular English case.
  For instance, `Device_Receive` is better than `Device_receive` or `DeviceReceive`
- Use a noun-verb form for naming functions and variables. For instance,
  `Device_Receive` is better than `Receive_Device`. Use variable naming as a
  primitive form of namespace, applying hierarchical order.
- Make variable names descriptive. Names like `modX` or `d_ab` are confusing and
  waste programmer time. Don't be afraid of longer variable names. The one
  exception is the use of `i` in very short `for` loops, which is a matter of
  convention.
- Local variables can use lowercase, while globals should be named similarly
  to functions, with scoping considered. This is important to avoid the pitfall
  of global namespace clutter, and distinguish variable scope.
- `enum` and `struct` typedefs should always have a `_t` at the end.



# Code Commenting

Code commenting is an extremely important practice, especially working in teams.
There is nuance to what makes for good comments, and the most difficult part is
the consistent application of principles.

- Avoid comments that just say the code. Don't make comments like _Set variable
  to be equal to 5_ or _Set GPIO pin to high_! If your comment takes longer to
  read than the code, it's a bad comment.
- Make comments that describe intention, as this can be used to verify if code
  is doing what it's supposed to. Guessing the original authorial intention is
  a frustrating experience.
- Make comments that describe a few lines of code at a time, usually between one
  to six lines of code. In the case of configuration, more is acceptable.
- When making TODO comments, strictly use the form "// TODO" or "//TODO" as this
  makes todo actions searchable.
- Typically avoid comments at the end of code lines. Use them to describe some
  technical detail of a line if it is not immediately obvious, and only when
  it is necessary.



# Closing Note

For almost every rule of practice, careful judgment can find exceptions where
compliance leads to worse code. This should be rare, and done with careful
judgment, but keeping in mind that deliberating on small things can be a waste
of time as well.
