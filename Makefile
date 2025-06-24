NAME = woody_woodpacker

CXX = cc
CXXFLAGS = -Wall -Wextra -Werror -std=c99
SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = main.c elf_parser.c encryption.c packer.c stub.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)
HEADERS = $(INCDIR)/woody.h

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)
	rm -f woody

re: fclean all

test: $(NAME)
	$(CXX) -m64 -o test_sample resources/sample.c
	./$(NAME) test_sample
	./woody
	rm -f test_sample

.PHONY: all clean fclean re test
