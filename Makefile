SRC	= src/main.c \
	src/network/ntp.c \
	src/network/packet.c \

OBJ	= $(SRC:.c=.o)

NAME = ntp-tunnel

RM = rm -f

CC = gcc

CFLAGS = -Wall
CFLAGS += -I./include

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re