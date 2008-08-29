package org.diracvideo.Schroedinger;

/** Synchronized Picture Queue. */

class Queue {
    Node free, head, tail;
    public Queue(int n) {
	free = head = tail = null;
	for(int i = 0; i < n; i++) {
	    Node nd = new Node();
	    nd.next = free;
	    free = nd;
	}
    }

    public synchronized boolean cyclic() {
	if(head == null)
	    return false;
	Node bunny = head.next, turtle = head;
	while(bunny != null &&
	      turtle != null) {
	    if(bunny == turtle) 
		return true;
	    turtle = turtle.next;
	    bunny = bunny.next;
	    if(turtle == bunny) 
		return true;
	    if(bunny != null)
		bunny = bunny.next;
	}
	return false; 
    }

    public synchronized void remove(int n) {
	Node pr = null;
	for(Node nd = head; nd != null; nd = nd.next) {
	    if(nd.load.num == n) {
		if(pr == null) 
		    head = head.next;
		else 
		    pr.next = nd.next;
		nd.next = free;
		free = nd;
		break;
	    }
	    pr = nd;
	}
    }

    public synchronized void add(Picture p) {
	Node nd;
	if(full()) {
	    nd = head;
	    head = head.next;
	    tail.next = nd;
	    tail = nd;
	} else {
	    nd = free;
	    free = free.next;
	    if(empty()) {
		head = tail = nd;
	    } else {
		tail.next = nd;
		tail = nd;
	    }
	}
	nd.load = p;
	nd.next = null;
    }

    public synchronized Picture get(int n)  {
	for(Node nd = head; nd != null; nd = nd.next) {
	    if(nd.load.num == n)
		return nd.load;
	}
	return null;
    }
    
    public synchronized Picture pop() {
	Node nd = head;
	Picture pic = nd.load;
	if(pic == null) {
	    System.err.println("Load = null");
	    System.exit(1);
	}
	head = head.next;
	nd.next = free;
	free = nd;
	nd.load = null;
	return pic;
    }

    public boolean full() {
	return free == null;
    }

    public boolean empty() {
	return head == null;
    }

    public synchronized void flush() {
	if(head != null) {
	    tail.next = free;
	    free = head;
	    head = tail = null;
	}
    }
}

class Node {
    Picture load;
    Node next;
}