int main() {
    int a = 0;
    int b = 1;
    int ret = 0;
    switch (a) {
    case 0: {
        switch (b) {
        case 0: {
            ret += 1;
            break;
        }
        case 1: {
            ret += 2; 
            break;
        }
        default: {
            ret += 4;
            break;
        }
        }

        break;
    }
    case 1: {
        ret += 8;
        break;
    }
    default: {
        ret += 16;
        break;
    }
    }

    return ret;
}
