package org.diracvideo.Schroedinger;

/** Synchronized Picture Queue. */

public class Queue {
    private Node free, head, tail;
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
	Node nd, pr = null;
	for(nd = head; nd != null; nd = nd.next)  {
	    if(nd.load.num == n) {
		if(pr == null)
		    head = head.next;
		else 
		    pr.next = nd.next;
		nd.next = free;
		nd.load = null;
		free = nd;
		break;
	    }
	    pr = nd;
	}
    }

    public synchronized void add(Picture p) {
	Node nd;
	if(full())  {
	    nd = head;
	    head = head.next;
	} else {
	    nd = free;
	    free = free.next;
	}
	if(empty()) {
	    head = tail = nd;
	} else {
	    tail.next = nd;
	    tail = nd;
	}
	nd.next = null;
	nd.load = p;
    }

    public synchronized Picture pop() {
	if(empty())
	    return null;
	Node nd = head;
	head = head.next;
	Picture pic = nd.load;
	nd.load = null;
	nd.next = free;
	free = nd;
	return pic;
    }

    public synchronized void flush() {
	if(head != null) {
	    tail.next = free;
	    free = head;
	    head = tail = null;
	}
    }

    public synchronized Picture get(int n)  {
	for(Node nd = head; nd != null; nd = nd.next) {
	    if(nd.load.num == n)
		return nd.load;
	}
	return null;
    }
    


    public boolean full() {
	return free == null;
    }

    public boolean empty() {
	return head == null;
    }

    public String toString() {
	StringBuilder sb = new StringBuilder();
	sb.append("org.diracvideo.Schroedinger.Queue");
	int i = 0;
	for(Node nd = head; nd != null; nd = nd.next) {
	    if((i++ % 8) == 0)
		sb.append("\n");
	    sb.append(String.format("%d -> ", nd.load.num));
	}
	sb.append("null");
	return sb.toString();
    }
}

class Node {
    Picture load;
    Node next;
}