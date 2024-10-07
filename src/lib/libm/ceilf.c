float ceilf(float x) {
    long lx = x;
    if (lx >= 0 && lx < x)
        return (float)(lx + 1);
    else
        return (float)lx;
}
