package com.example.unix_omok.repository;

import com.example.unix_omok.OmokData;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface OmokDataRepository {
    List<OmokData> findAll();
}
